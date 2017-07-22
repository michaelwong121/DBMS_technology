#include "join.h"

buffer::buffer(short t, int cap){
	type = t;
	fp = NULL;

	capacity = cap;
	in_buf = 0;
	current_pos = -1;

	buf = new record[cap];
}

buffer::~buffer(){
	delete [] buf;
	if (fp){
		fclose(fp);
	}
}

int buffer::init_fp(char* file){
	//char *file_type = (char *)((type == INPUT)? "rb" : "wb");

	if (!(fp = fopen(file, "rb"))){
		cerr << "Could not open file '" << file << "'\n";
		return (FAILURE);
	}

	// fill buffer after first init
	in_buf = fread(buf, sizeof(record), capacity, fp);

	return SUCCESS;
}

void buffer::reset_buf(){
	rewind(fp);// rewind file ptr

	// fill buffer after rewinding
	in_buf = fread(buf, sizeof(record), capacity, fp);

	current_pos = -1;
}

void buffer::print_buf(){
	for (int i = 0; i < capacity; i++){
		if (buf[i].UID1) cout << "UID: " << buf[i].UID1 << ", deg: " << buf[i].UID2 << '\n';
	}
}

// this function will return 1 record through the parameters
// and return the status of the read.
// It will automatically update the buffer if it reaches the end
int buffer::read_from_buffer(record &r){
	if (type != INPUT) return FAILURE;

	// advance pointer first
	current_pos++;
	if (current_pos >= in_buf){
		if (in_buf != capacity) return EMPTY;

		current_pos = 0; //wrap around
		in_buf = fread(buf, sizeof(record), capacity, fp);
	}

	// this should copy
	r = buf[current_pos];

	return SUCCESS;
}


top_celeb_list::top_celeb_list(int capacity){
	list = new celeb[capacity]();
	num_in_list = capacity;
}

top_celeb_list::~top_celeb_list(){
	delete [] list;
}

void top_celeb_list::replace_celeb(celeb c){
	// replace last item in list if new item is bigger than last item
	if ((c.in_deg - c.out_deg) > (list[num_in_list - 1].in_deg - list[num_in_list - 1].out_deg)){
		list[num_in_list - 1] = c;
	}

	// then sort the list
	qsort(list, num_in_list, sizeof(celeb), compare_fame);
}

void top_celeb_list::print_list(){
	for (int i = 0; i < num_in_list; i++){
		if (list[i].UID){
			cout << "UID: " << list[i].UID << ", in deg: " << list[i].in_deg
				 << ", out deg: " << list[i].out_deg << '\n';
		}
	}
}

int compare_fame(const void *a, const void *b){
	int a_f = ((const celeb*)a)->in_deg - ((const celeb*)a)->out_deg;
	int b_f = ((const celeb*)b)->in_deg - ((const celeb*)b)->out_deg;
	return (b_f - a_f);
}

// binary search. Return index of found record
int bin_search(record *buf, int start, int end, int uid2){
	if (start > end){
		return -1;
	}

	int m = start + ((end - start) / 2);
	if (buf[m].UID1 == uid2){
		return m;
	}
	else if (buf[m].UID1 > uid2){
		return bin_search(buf, start, m - 1, uid2);
	}

	return bin_search(buf, m + 1, end, uid2);
}

// need to scan forward or backward because of duplicate
// Return index of the found record, or -1
int scan_for_record(record *buf, int capacity, int index, record s){
	int forward_index = index;

	while (buf[forward_index].UID1 == s.UID2 && forward_index < capacity){
		if (buf[forward_index].UID2 == s.UID1 && s.UID1 > s.UID2){
			return forward_index;
		}
		forward_index++;
	}

	int backward_index = index;
	while (buf[backward_index].UID1 == s.UID2 && backward_index >= 0){
		if (buf[backward_index].UID2 == s.UID1 && s.UID1 > s.UID2){
			return backward_index;
		}
		backward_index--;
	}

	return -1;
}

