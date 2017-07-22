#include "merge.h"

// total merge wrapper
int ext_merge_sort (char *in_file, char *out_file, long total_mem, long blocksize, short col_id) {

	long records_per_ram = total_mem / sizeof(record);
	long records_per_block = blocksize / sizeof(record);

	FILE *fp_read, *fp_write;
	if (!(fp_read = fopen(in_file, "rb"))){
		cerr << "Could not open file '" << in_file << "' for reading\n";
		return (FAILURE);
	}

	// find the number of records
	fseek(fp_read, 0L, SEEK_END);
	long length = ftell(fp_read);
	rewind(fp_read);// rewind inFile

	// check if there is enough memory
	// need the divide 2 because of splitting the partition into two because of qsort
	if (length > (total_mem / 2) * (total_mem / (2 * blocksize) - 1)){
		cerr << "Not enough memory.\n";
		return (FAILURE);
	}


	long p1_buf_size = records_per_ram / 2; // split the partition into two because qsort uses too much mem
	record *ram_buf = new record[p1_buf_size]; // buffer that represents ram

	// Phase 1
	int in_buf = 0;
	int no_of_runs = 0; // record the number of runs in total
	char out_filename[MAX_PATH_LENGTH]; // write to a different file per run
	while ((in_buf = fread(ram_buf, sizeof(record), p1_buf_size, fp_read))){
		++no_of_runs;
		sprintf(out_filename, "sublist%d.dat", no_of_runs);
		if (!(fp_write = fopen(out_filename, "wb"))){
			cerr << "Could not open file '" << out_filename << "' for writing\n";
			return (FAILURE);
		}

		if (col_id == UID_ONE){
			qsort(ram_buf, in_buf, sizeof(record), compare_uid1);
		}
		else {
			qsort(ram_buf, in_buf, sizeof(record), compare_uid2);
		}
		fwrite(ram_buf, sizeof(record), in_buf, fp_write);
		fclose(fp_write);
	}

	fclose(fp_read);

	delete[] ram_buf; // can free it because phase 2 will create its own buf's

	MergeManager manager;

	// Loop through all input files and fill-in initial buffers
	// initialize all fields according to the input and the results of Phase I
	if (manager.init_merge(no_of_runs, records_per_ram, records_per_block, out_file, col_id)!= SUCCESS){
		return FAILURE;
	}

	// Phase 2
	return manager.merge_runs ();
}


//manager fields should be already initialized in the caller
int MergeManager::merge_runs (){
	int  result; //stores SUCCESS/FAILURE returned at the end	
	
	FILE *count_fp;

	if (col_id == UID_ONE){
		if (!(count_fp = fopen("outdegree.dat", "wb"))){
			cerr << "Could not open file 'outdegree.dat' for writing\n";
			return FAILURE;
		}
	}
	else{
		if (!(count_fp = fopen("indegree.dat", "wb"))){
			cerr << "Could not open file 'indegree.dat' for writing\n";
			return FAILURE;
		}
	}

	int this_uid = 0;
	int this_uid_follows = 0;
	record output_r;

	while (current_heap_size > 0) { //heap is not empty
		HeapElement smallest;
		record next; //here next comes from input buffer
		
		if(get_top_heap_element (&smallest)!=SUCCESS)
			return FAILURE;

		result = get_next_input_element (smallest.run_id, next);
		
		if (result==FAILURE)
			return FAILURE;

		if(result==SUCCESS) {//next element exists, may also return EMPTY
			if(insert_into_heap (smallest.run_id, next)!=SUCCESS)
				return FAILURE;
		}

		// Write count into outdegree.dat or indegree.dat
		int uid = (col_id == UID_ONE)? smallest.UID1 : smallest.UID2;
		// a new person
		if (uid != this_uid){
			if (this_uid != 0){
				// first write old person to disk.
				output_r.UID1 = this_uid; 		  // use UID1 to store uid
				output_r.UID2 = this_uid_follows; // use UID2 to store count
				fwrite(&output_r, sizeof(record), 1, count_fp);
			}

			// then go to next person
			this_uid = uid;
			this_uid_follows = 1;
		}
		// same person
		else {
			++this_uid_follows;
		}

		//cout<< "Putting record: "<<smallest.UID2<<'\n';
		output_buffer [current_output_buffer_position].UID1=smallest.UID1;
		output_buffer [current_output_buffer_position].UID2=smallest.UID2;
		
		current_output_buffer_position++;

        //staying on the last slot of the output buffer - next will cause overflow
		if(current_output_buffer_position == output_buffer_capacity ) {
			if(flush_output_buffer()!=SUCCESS) {
				return FAILURE;			
			}	
			current_output_buffer_position=0;
		}
	
	}

	// write last person
	output_r.UID1 = this_uid; 		  // use UID1 to store uid
	output_r.UID2 = this_uid_follows; // use UID2 to store count
	fwrite(&output_r, sizeof(record), 1, count_fp);
	fclose(count_fp);

	
	//flush what remains in output buffer
	if(current_output_buffer_position > 0) {
		if(flush_output_buffer()!=SUCCESS)
			return FAILURE;
	}
	
	return SUCCESS;	
}


//4. returns top heap element, rearranges heap nodes
int MergeManager::get_top_heap_element (HeapElement * result){
	HeapElement item;
	int child, parent;

	if(current_heap_size == 0){
		printf( "UNEXPECTED ERROR: popping top element from an empty heap\n");
		return FAILURE;
	}

	*result=heap[0];  //to be returned

	//now we need to reorganize heap - keep the smallest on top
	item = heap [--current_heap_size]; // to be reinserted

	parent = 0;
	while ((child = (2 * parent) + 1) < current_heap_size) {
		// if there are two children, compare them 
		if (child + 1 < current_heap_size &&
		   (compare_heap_elements(heap[child],heap[child + 1])>0))
			++child;
		
		// compare item with the larger 
		if (compare_heap_elements(item, heap[child])>0) {
			heap[parent] = heap[child];
			parent = child;
		} 
		else 
			break;
	}
	heap[parent] = item;
	
	return SUCCESS;
}

//5. inserts new element into heap, rearranges nodes to keep smallest on top
int MergeManager::insert_into_heap (int run_id, record &input){

	HeapElement new_heap_element;
	int child, parent;

	new_heap_element.UID1 = input.UID1;
	new_heap_element.UID2 = input.UID2;
	new_heap_element.run_id = run_id;
	
	if (current_heap_size == heap_capacity) {
		printf( "Unexpected ERROR: heap is full\n");
		return FAILURE;
	}
  	
	child = current_heap_size++; /* the next available slot in the heap */
	
	while (child > 0) {
		parent = (child - 1) / 2;
		if (compare_heap_elements(heap[parent],new_heap_element)>0) {
			heap[child] = heap[parent];
			child = parent;
		} 
		else 
			break;
	}
	heap[child]= new_heap_element;
	return SUCCESS;
}


// 0. Constructor for MergeManager class: preset everything to 0 or nullptr
MergeManager::MergeManager(){
	col_id = 0;

	heap = NULL;
	current_heap_size = 0;
	heap_capacity = 0;

	inputFP = NULL;
	input_file_numbers = NULL;
	input_buffers = NULL;
	input_buffer_capacity = 0;
	current_input_file_positions = NULL;
	current_input_buffer_positions = NULL;
	total_input_buffer_elements = NULL;

	outputFP = NULL;
	output_buffer = NULL;
	current_output_buffer_position = 0;
	output_buffer_capacity = 0;
}



// 2. creates and fills initial buffers, initializes heap taking
//    1 top element from each buffer
int MergeManager::init_merge (int no_of_run, long records_per_ram, long records_per_block, char* out_file, short col) {

	col_id = col;

	heap = new HeapElement[no_of_run];
	heap_capacity = no_of_run;

	strcpy(input_prefix, "sublist");
	input_file_numbers = new int[no_of_run];
	for (int i = 0; i < no_of_run; ++i){
		input_file_numbers[i] = i + 1; // the run id are from 1 to no_of_run
	}

	int capacity = records_per_ram / (no_of_run + 1); // 1 for output buf
	capacity = records_per_block * (capacity / records_per_block); // force to align with block
	input_buffer_capacity = capacity;
	input_buffers = new record*[no_of_run];
	for (int j = 0; j < no_of_run; ++j){
		input_buffers[j] = new record[input_buffer_capacity];
	}

	current_input_file_positions = new int[no_of_run](); // init to all 0
	current_input_buffer_positions = new int[no_of_run](); // init to all 0
	total_input_buffer_elements = new int[no_of_run](); // init to all 0

	strcpy(output_file_name, out_file);
	if (!(outputFP = fopen(output_file_name, "wb"))){
		cerr << "Could not open file '" << output_file_name << "' for writing\n";
		return (FAILURE);
	}
	output_buffer_capacity = records_per_ram - no_of_run * input_buffer_capacity;
	output_buffer_capacity = records_per_block * (output_buffer_capacity / records_per_block); // force to align with block
	output_buffer = new record[output_buffer_capacity];

	// fill input buffers
	int result1, result2;
	for (int run_id = 1; run_id <= no_of_run; ++run_id){
		result1 = refill_buffer(run_id);
		if (result1 == FAILURE) return FAILURE;
		else if(result1 == SUCCESS){
			// initialize heap
			record r;
			result2 = get_next_input_element(run_id, r);
			if (result2 == FAILURE) return FAILURE;
			else if (result2 == SUCCESS) {
				if (insert_into_heap(run_id, r) == FAILURE) return FAILURE;
			}
		}
	}

	//print_all_input_buf();
	//print_heap ();
	return SUCCESS;// do nothing if EMPTY is returned
}

// 3. flushes output buffer to disk when full
//    The output buffer position is reset in merge_runs after this is called.
int MergeManager::flush_output_buffer () {

	fwrite(output_buffer, sizeof(record), current_output_buffer_position, outputFP);

	return SUCCESS;
}

//6. reads next element from an input buffer to transfer it to the heap.
//   Uploads records from disk if all elements are processed
int MergeManager::get_next_input_element( int run_id, record &result) {

	int buf_pos = run_id - 1;

	if (!total_input_buffer_elements[buf_pos] && current_input_file_positions[buf_pos] == -1){
		return EMPTY;
	}

	record &r = input_buffers[buf_pos][current_input_buffer_positions[buf_pos]];
	result.UID1 = r.UID1;
	result.UID2 = r.UID2;

	total_input_buffer_elements[buf_pos]--;
	if (!total_input_buffer_elements[buf_pos]){
		// don't need to refill buffer if file is done reading
		// need to return SUCCESS or else wouldn't be added into the heap
		if (current_input_file_positions[buf_pos] == -1){
			return SUCCESS;
		}

		// otherwise, refill buffer
		return refill_buffer(run_id);
	}
	else {
		current_input_buffer_positions[buf_pos]++;
	}

	return SUCCESS;
}

//7. refills input buffer from the corresponding run
int MergeManager::refill_buffer (int run_id) {

	int buffer_position = run_id - 1;
	int next_element_position = current_input_file_positions[buffer_position];
	if (next_element_position == -1){
		return EMPTY;
	}

	char in_filename[MAX_PATH_LENGTH];
	sprintf(in_filename, "%s%d.dat", input_prefix, run_id);
	if (!(inputFP = fopen(in_filename, "rb"))){
		cerr << "Could not open file '" << in_filename << "' for reading\n";
		return(FAILURE);
	}

	fseek(inputFP, next_element_position * sizeof(record), SEEK_SET);

	int in_buf = fread(input_buffers[buffer_position], sizeof(record), input_buffer_capacity, inputFP);
	if (in_buf != input_buffer_capacity){ // reach the end
		current_input_file_positions[buffer_position] = -1;
	}
	else {
		// cout<< "incrememt " << run_id<<" by: "<<input_buffer_capacity <<'\n';
		current_input_file_positions[buffer_position] += input_buffer_capacity;
	}
	total_input_buffer_elements[buffer_position] = in_buf;
	current_input_buffer_positions[buffer_position] = 0;

	fclose(inputFP);

	return SUCCESS;
}

//8. Use destructor to frees all dynamically allocated memory
MergeManager::~MergeManager() {
	delete [] heap;
	delete [] input_file_numbers;

	// heap capacity == no_of_runs
	for (int i = 0; i < heap_capacity; ++i){
		delete [] input_buffers[i];
	}
	delete [] input_buffers;
	delete [] current_input_file_positions;
	delete [] current_input_buffer_positions;
	delete [] total_input_buffer_elements;
	
	delete [] output_buffer;
	fclose(outputFP);
}


//9. Application-specific comparison function
int MergeManager::compare_heap_elements (HeapElement &a, HeapElement &b) {
	int a_f = (col_id == UID_ONE) ? a.UID1 : a.UID2;
	int b_f = (col_id == UID_ONE) ? b.UID1 : b.UID2;
	return (a_f - b_f);
}


// util functions
void MergeManager::print_input_buf (int run_id){
	int buf_pos = run_id - 1;

	cout<< "run id: " << run_id << ", elements in buf: " << total_input_buffer_elements[buf_pos];
	cout<< ", buf capacity: "<<input_buffer_capacity<< '\n';
	cout<< "current buf position: "<<current_input_buffer_positions[buf_pos];
	cout<< ", current file position: "<<current_input_file_positions[buf_pos]<<'\n';
	cout<< "buf:";
	for (int i = 0; i < input_buffer_capacity; ++i){
		cout<<" [" << input_buffers[buf_pos][i].UID2 << "]";
	}
	cout<<'\n';
}

void MergeManager::print_all_input_buf (){
	for (int k = 1; k <= heap_capacity; ++k){
		print_input_buf(k);
	}
}

void MergeManager::print_heap (){
	cout<<"heap:";
	for (int i = 0; i < current_heap_size; ++i){
		cout<<" [" << heap[i].UID2<<"]";
	}
	cout<<'\n';
}



// other functions
int compare_uid1 (const void *a, const void *b) {
	int a_f = ((const record*)a)->UID1;
	int b_f = ((const record*)b)->UID1;
	return (a_f - b_f);
}
int compare_uid2 (const void *a, const void *b) {
	int a_f = ((const record*)a)->UID2;
	int b_f = ((const record*)b)->UID2;
	return (a_f - b_f);
}

