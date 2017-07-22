#include "join.h"
#include "merge.h"
#include <sys/timeb.h>


int main (int argc, char **argv) {

	//process and validate command-line arguments
	if(argc != 4) {
		cerr << "Usage: top_celeb <input filename> <total mem> "
			 << "<block size>\n";
		exit(FAILURE);
	}

	char * filename = argv[1];
	long total_mem = atoi(argv[2]); // in bytes
	long blocksize = atoi(argv[3]);
	long records_per_block = blocksize / sizeof(record);
	long blocks_per_ram = total_mem / blocksize;

	struct timeb t_begin, t_end;

	ftime(&t_begin);

	char sort_file1[MAX_PATH_LENGTH];
	// sort and count the in and out degree of each person
	strcpy(sort_file1, "sorted_by_uid1.dat");
	if (ext_merge_sort(filename, sort_file1, total_mem, blocksize, UID_ONE) != SUCCESS){
		cerr << "Sorting error, uid1\n";
		exit(FAILURE);
	}

	char sort_file2[MAX_PATH_LENGTH];
	strcpy(sort_file2, "sorted_by_uid2.dat");
	if  (ext_merge_sort(filename, sort_file2, total_mem, blocksize, UID_TWO) != SUCCESS){
		cerr << "Sorting error, uid2\n";
		exit(FAILURE);
	}

	// do a join
	// replace min of list when joining
	top_celeb_list top_list(10);

	// since we know that outdegree.dat and indegree.dat are sorted, can do zigzag join
	record r;
	record s;


	char out_file[MAX_PATH_LENGTH];
	strcpy(out_file, "outdegree.dat");
	buffer R(INPUT, (blocks_per_ram / 2) * records_per_block);
	short r_result = R.init_fp(out_file);
	if (r_result == FAILURE){
		cerr << "Reading error. Will exit program.\n";
		exit(FAILURE);
	}

	char in_file[MAX_PATH_LENGTH];
	strcpy(in_file, "indegree.dat");
	buffer S(INPUT, (blocks_per_ram / 2) * records_per_block);
	short s_result = S.init_fp(in_file);
	if (s_result == FAILURE){
		cerr << "Reading error. Will exit program.\n";
		exit(FAILURE);
	}

	r_result = R.read_from_buffer(r);
	s_result = S.read_from_buffer(s);
	celeb c;
	while (r_result == SUCCESS && s_result == SUCCESS){
		if (r.UID1 == s.UID1){
			c.UID = r.UID1;
			c.out_deg = r.UID2;
			c.in_deg = s.UID2;
			//cout << "UID: " << c.UID << ", in deg: " << c.in_deg
			//	 << ", out deg: " << c.out_deg << '\n';
			top_list.replace_celeb(c);
			r_result = R.read_from_buffer(r);
			s_result = S.read_from_buffer(s);
		}
		else if (r.UID1 > s.UID1){
			c.UID = s.UID1;
			c.out_deg = 0; // pad with 0
			c.in_deg = s.UID2;
			top_list.replace_celeb(c);
			s_result =  S.read_from_buffer(s);
		}
		else {
			r_result = R.read_from_buffer(r);
		}
	}
	if (r_result == FAILURE || s_result == FAILURE){
		cerr << "Error reading from buffer R. Will exit program.\n";
		exit(FAILURE);
	}

	top_list.print_list();

	ftime(&t_end);

	long time_spent = (long) (1000 * (t_end.time - t_begin.time) + (t_end.millitm - t_begin.millitm));
	cout << "Time spent: " << time_spent <<'\n';

	return 0;
}
