#include "join.h"
#include <sys/timeb.h>

int main (int argc, char **argv) {

	//process and validate command-line arguments
	if(argc != 4) {
		cerr << "Usage: find_true_friend <input filename> <total mem> "
			 << "<block size>\n";
		exit(FAILURE);
	}

	char * filename = argv[1];
	long total_mem = atoi(argv[2]); // in bytes
	long blocksize = atoi(argv[3]);
	long records_per_block = blocksize / sizeof(record);
	long blocks_per_ram = total_mem / blocksize;

	short result;

	struct timeb t_begin, t_end;

	ftime(&t_begin);

	char sort_file1[MAX_PATH_LENGTH];
	// sort the file
	strcpy(sort_file1, "sorted_by_uid1.dat");
	if (ext_merge_sort(filename, sort_file1, total_mem, blocksize, UID_ONE) != SUCCESS){
		cerr << "Sorting error, uid1\n";
		exit(FAILURE);
	}

	// file descriptor for relation R
	FILE *fp_R;
	if (!(fp_R = fopen(sort_file1, "rb"))){
		cerr << "Could not open file '" << sort_file1 << "'\n";
		return (FAILURE);
	}

	// create a buffer for reading from R with size M - 1 blocks
	int R_cap = (blocks_per_ram - 1) * records_per_block; // capacity for relation R
	record *R_buf = new record [R_cap];
	int in_buf;


	// 1 block for inner loop
	buffer S(INPUT, 1 * records_per_block);
	result = S.init_fp(filename);
	if (result == FAILURE){
		cerr << "Reading error. Will exit program.\n";
		exit(FAILURE);
	}

	long true_friend_count = 0;

	while ((in_buf = fread(R_buf, sizeof(record), R_cap, fp_R))){ // should be sorted
		record s;
		S.reset_buf();
		short S_result;
		while ((S_result = S.read_from_buffer(s)) == SUCCESS){
			// binary search
			int index = bin_search(R_buf, 0, in_buf - 1, s.UID2);
			if (index != -1){
				// scan forward and backward
				int ind = scan_for_record(R_buf, in_buf, index, s);
				if (ind != -1){
					//cout << "UID1: " << R_buf[ind].UID1 << ", UID2: "
					//	 << R_buf[ind].UID2 <<'\n';
					true_friend_count++;
				}
			}
		}
		if (S_result == FAILURE){
			cerr << "Error reading from buffer S. Will exit program.\n";
			exit(FAILURE);
		}
	}

	cout << "Total true friend count: " << true_friend_count << '\n';

	delete [] R_buf;
	fclose(fp_R);

	ftime(&t_end);

	long time_spent = (long) (1000 * (t_end.time - t_begin.time) + (t_end.millitm - t_begin.millitm));
		cout << "Time spent: " << time_spent <<'\n';

	return 0;
}
