#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <sys/timeb.h>
#include "utils.h"
#include <stdio.h>
#include <string.h>

using namespace std;

/*
 * Move into X random locations in the binary file, and override each
 * record by itself.
 */
int main(int argc, char* argv[]) {

	if(argc != 3) {
        cerr << "Usage: write_blocks_rand <input filename> "
        	 <<	"<number of iterations>\n";
        exit(1);
	}

	char * filename = argv[1];
	int iterations = atoi(argv[2]);

	FILE *fp;

	if (!(fp = fopen(filename, "rb+"))){
		cerr << "Could not open file '" << filename << "' for reading\n";
		exit(1);
	}

	struct timeb t_begin, t_end;

	// find the number of records
	// necessary because need to give a range for the random number
	fseek(fp, 0L, SEEK_END);
	long length = ftell(fp);
	long total_records = length / sizeof(record); // total number of records

	// rewind inFile
	rewind(fp);

	// create block buffer
	record r;
	srand(time(NULL));

	ftime(&t_begin);
	for (int i = 0; i < iterations; ++i){
		// randomly generate a number from 0 to total_records - 1
		long rand_start = rand() % total_records;

		// read into buffer
		fseek(fp, rand_start * sizeof(record), SEEK_SET);
		int result = fread((char *) &r, sizeof(record), 1, fp);
		if (result != 1){
			cerr << "Reading error\n";
			exit(1);
		}

		// overwrite with (1,2)
		r.uid1 = 1;
		r.uid2 = 2;

		// write back to binary
		fseek(fp, rand_start * sizeof(record), SEEK_SET);
		fwrite((char *) &r, sizeof(record), 1, fp);
	}
	ftime(&t_end);
	fclose(fp);


	// time elapsed in milliseconds
	long time_spent_ms = (long) (1000 *(t_end.time - t_begin.time)
		  		       + (t_end.millitm - t_begin.millitm));

	float rate = ((iterations *sizeof(record))/(float)time_spent_ms * 1000)/MB;

	// result in MB per second
	cout << "Data rate: " << fixed << setprecision(3) << rate << " MBPS\n";

    return 0;
}
