#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sys/timeb.h>
#include "utils.h"
#include <stdio.h>
#include <string.h>

using namespace std;

/*
 * Implement the max and average query, that calculates max and average
 * by sampling data in X random positions of the data file (X is provided
 * as an additional command-line parameter). The program performs a loop
 * with X iterations. In each iteration it reads a block of records into a
 * single-page buffer - starting from a random position in a file - and
 * examines all the records in this block for max and average.
 */
int main(int argc, char* argv[]) {

	if(argc != 4) {
        cerr << "Usage: read_blocks_rand <input filename> <blocksize> "
        	 <<	"<number of iterations>\n";
        exit(1);
	}

	char * filename = argv[1];
	int blocksize = atoi(argv[2]);
	int records_per_block = blocksize / sizeof(record);
	int iterations = atoi(argv[3]);

	FILE *fp_read;

	if (!(fp_read = fopen(filename, "rb"))){
		cerr << "Could not open file '" << filename << "' for reading\n";
		exit(1);
	}

	struct timeb t_begin, t_end;

	// find the number of records
	// necessary because need to give a range for the random number
	fseek(fp_read, 0L, SEEK_END);
	long length = ftell(fp_read);
	long total_records = length / sizeof(record); // total number of records

	// rewind inFile
	rewind(fp_read);

	// create block buffer
	record *buffer = new record[records_per_block];

	int counted_records = 0;    // total number of counted records
	int total_accounts = 0;     // total number of accounts counted
	int max_follow = 0;         // max
	srand(time(NULL));

	ftime(&t_begin);
	for (int i = 0; i < iterations; ++i){
		int this_uid1 = 0;          // uid1 of current person
		int this_uid1_follows = 0;  // how many people this person is following

		// randomly generate a number from 0 to total_records - 1
		long rand_start = rand() % total_records;
		//cout<< "rand start: "<<rand_start<<'\n';
		fseek(fp_read, rand_start * sizeof(record), SEEK_SET);

		long in_buf = fread((char *) buffer, sizeof(record), records_per_block, fp_read);
		for (int j = 0; j < in_buf; ++j){
			record &r = buffer[j];
			//cout<<"r "<<r.uid1<< " "<<r.uid2<<'\n';
			// a new person
			if (r.uid1 != this_uid1){
				max_follow = max(this_uid1_follows, max_follow);
				this_uid1 = r.uid1;
				this_uid1_follows = 1;
				++total_accounts;
			}
			// same person
			else {
				++this_uid1_follows;
			}
		}
		// need to do it for the last person
		max_follow = max(this_uid1_follows, max_follow);

		counted_records += in_buf;
	}
	ftime(&t_end);

	delete (buffer);
	fclose(fp_read);

	double average_follow = (double)counted_records / (double)total_accounts;

	cout << "max follow: " << max_follow << "\n";
	cout << "average follow: " << average_follow << '\n';


	// time elapsed in milliseconds
	long time_spent_ms = (long) (1000 *(t_end.time - t_begin.time)
		  		       + (t_end.millitm - t_begin.millitm));

	float rate = ((counted_records*sizeof(record))/(float)time_spent_ms * 1000)/MB;

	// result in MB per second
	cout << "Data rate: " << fixed << setprecision(3) << rate << " MBPS\n";

    return 0;
}
