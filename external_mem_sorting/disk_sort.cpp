#include "merge.h"


int main (int argc, char **argv) {

	//process and validate command-line arguments
	if(argc != 4) {
		cerr << "Usage: disk_sort <input filename> <total mem> "
			 << "<block size>\n";
		exit(FAILURE);
	}
	
	char out_file[MAX_PATH_LENGTH];
	strcpy(out_file, "output.dat");

	return ext_merge_sort (argv[1], out_file, atoi(argv[2]), atoi(argv[3]), UID_TWO);
}
