#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sys/timeb.h>
#include "merge.h"
#include <stdio.h>
#include <string.h>

using namespace std;

int main(int argc, char* argv[]) {

	if(argc != 5) {
        cerr << "Usage: distribution <input filename> <blocksize> <column id> <max degree>\n";
        exit(1);
	}

	char * filename = argv[1];
	int blocksize = atoi(argv[2]);
	int records_per_block = blocksize / sizeof(record);
	int col_id = atoi(argv[3]);
	int max_deg = atoi(argv[4]);
	int array_size = max_deg + 1; // +1 because need both 0 and max_deg

	if (col_id != UID_ONE && col_id != UID_TWO){
		cerr<< "column id has to be 1 or 2.\n";
		exit(1);
	}

	int *dist = new int[array_size]();

	FILE *fp_read, *fp_write;
	if (!(fp_read = fopen(filename, "rb"))){
		cerr << "Could not open file '" << filename << "' for reading\n";
		exit(1);
	}
	if (!(fp_write = fopen("historgram.csv", "w"))){
		cerr << "Could not open file 'histogram.csv' for writing\n";
		exit(1);
	}


	int this_uid = 0;          // uid of current person
	int this_uid_follows = 0;  // how many people this person is following

	// create block buffer
	record *buffer = new record[records_per_block];

	long in_buf = 0;
	while ((in_buf = fread((char *) buffer, sizeof(record), records_per_block, fp_read))){
		for (long i = 0; i < in_buf; ++i){
			int uid = (col_id == UID_ONE)? buffer[i].UID1 : buffer[i].UID2;
			// a new person
			if (uid != this_uid){
				dist[this_uid_follows]++;
				this_uid = uid;
				this_uid_follows = 1;
			}
			// same person
			else {
				++this_uid_follows;
			}
		}
	}
	dist[this_uid_follows]++;
	dist[0]--; // incremented one from the beginning, so need to decrement back

	delete(buffer);
	fclose(fp_read);

	//write to histogram.csv
	for (int k = 0; k < array_size; ++k){
		// write only non-zero degree
		if (dist[k]){
			fprintf(fp_write, "%d,%d\n", k, dist[k]);
		}
	}

	delete [] dist;


    return 0;
}





