#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sys/timeb.h>
#include "utils.h"
#include <stdio.h>
#include <string.h>

using namespace std;

/*
 *   Randomly overwrite X locations with the same record after the entire dataset
 *   has beenloaded into RAM.
 */
int main(int argc, char* argv[]) {

	if(argc != 3) {
        cerr << "Usage: write_ram_rand <input filename> <number of random locations>\n";
        exit(1);
	}

	char * filename = argv[1];
	int x = atoi(argv[2]);
	
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

	// allocate buffer for all the records
	record *ram_buf = new record[total_records];

	long result = fread((char *) ram_buf, sizeof(record), total_records, fp_read);
	if (result != total_records){
		cerr << "Reading error\n";
		exit(1);
	}
	
	srand(time(NULL));
	
	ftime(&t_begin);
	// overwriting the randomly chosen record by (1,2)
	for (int i = 0; i < x; ++i){
		long rand_pos = rand() % total_records;
		record &r = ram_buf[rand_pos];
		r.uid1 = 1;
		r.uid2 = 2;
	}
	ftime(&t_end);
	fclose(fp_read);

	// time elapsed in milliseconds
	long time_spent_ms = (long) (1000 *(t_end.time - t_begin.time)
		  		       + (t_end.millitm - t_begin.millitm));

	float rate = ((x*sizeof(record))/(float)time_spent_ms * 1000)/MB;

	// result in MB per second
	cout << "Data rate: " << fixed << setprecision(3) << rate << " MBPS\n";
	
	return 0;
} 
