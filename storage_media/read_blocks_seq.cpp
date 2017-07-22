#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sys/timeb.h>
#include "utils.h"
#include <stdio.h>
#include <string.h>

using namespace std;

/*
 *  Implement a query that reads the binary record file in blocks,
 *  and computes how many people each user followers at most and on average.
 *  The original records are sorted by uid1, which allows to implement this
 *  program with a single sequential pass over the binary file. The program
 *  outputs two values: max and average. The timing code should surround the
 *  entire calculation including reading each block from disk.
 */
int main(int argc, char* argv[]) {

	if(argc != 3) {
        cerr << "Usage: read_blocks_seq <input filename> <blocksize>\n";
        exit(1);
	}

	char * filename = argv[1];
	int blocksize = atoi(argv[2]);
	int records_per_block = blocksize / sizeof(record);
	FILE *fp_read;

	if (!(fp_read = fopen(filename, "rb"))){
		cerr << "Could not open file '" << filename << "' for reading\n";
		exit(1);
	}

	struct timeb t_begin, t_end;

	long this_uid1 = 0;          // uid1 of current person
	long this_uid1_follows = 0;  // how many people this person is following
	long total_accounts = 0;     // total number of accounts
	long max_follow = 0;         // max

	// find the number of records
	// necessary because need to give a range for the random number
	fseek(fp_read, 0L, SEEK_END);
	long length = ftell(fp_read);
	long total_records = length / sizeof(record); // total number of records

	// rewind inFile
	rewind(fp_read);

	// create block buffer
	record *buffer = new record[records_per_block];

	ftime(&t_begin);

	long in_buf = 0;
	while ((in_buf = fread((char *) buffer, sizeof(record), records_per_block, fp_read))){
		for (long i = 0; i < in_buf; ++i){
			record &r = buffer[i];
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
	}
	max_follow = max(this_uid1_follows, max_follow);
	ftime(&t_end);

	delete(buffer);
	fclose(fp_read);

	double average_follow = (double)total_records / (double)total_accounts;
	cout<<"max follow: "<<max_follow << "\n";
	cout<<"average follow: "<<average_follow << '\n';

	// time elapsed in milliseconds
	long time_spent_ms = (long) (1000 *(t_end.time - t_begin.time)
		  		       + (t_end.millitm - t_begin.millitm));

	float rate = ((total_records*sizeof(record))/(float)time_spent_ms * 1000)/MB;

	// result in MB per second
	cout << "Data rate: " << fixed << setprecision(3) << rate << " MBPS\n";

    return 0;
}
