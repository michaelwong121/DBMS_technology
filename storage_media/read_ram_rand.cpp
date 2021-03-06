#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <ctime>
#include <sys/timeb.h>
#include "utils.h"
#include <stdio.h>
#include <string.h>

using namespace std;

/*
 *   Read the entire binary file into a large in-memory buffer,
 *   and perform the same query. You will need to determine the file
 *   size in order to allocate the buffer of the appropriate size in
 *   your code. Time the program in bytes/second after all the records
 *   have been loaded into RAM.
 */
int main(int argc, char* argv[]) {

	if(argc != 4) {
		cerr << "Usage: read_ram_rand <input filename> <blocksize> "
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

	// allocate buffer for all the records
	record *ram_buf = new record[total_records];

	fread((char *) ram_buf, sizeof(record), total_records, fp_read);

	// get random number between 0 to total_records - 1;
	srand(time(NULL));

	long counted_records = 0;    // total number of counted records
	long total_accounts = 0;     // total number of accounts
	int max_follow = 0;         // max

	ftime(&t_begin);
	for (int i = 0; i < iterations; ++i){

		int this_uid1 = 0;          // uid1 of current person
		int this_uid1_follows = 0;  // how many people this person is following

		long rand_start = rand() % total_records;

		long end = min(rand_start + records_per_block, total_records);

		for (int j = rand_start; j < end; ++j){
			record &r = ram_buf[j];
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

		counted_records += (end - rand_start);
	}
	ftime(&t_end);

	delete (ram_buf);
	fclose(fp_read);

	double average_follow = (double)counted_records / (double) total_accounts;
	cout<<"max follow: "<<max_follow << "\n";
	cout<<"average follow: "<<average_follow << '\n';

	// time elapsed in milliseconds
	long time_spent_ms = (long) (1000 *(t_end.time - t_begin.time)
		  		       + (t_end.millitm - t_begin.millitm));

	float rate = ((counted_records*sizeof(record)) * 1000)/MB;
	rate = rate/time_spent_ms;

	// result in MB per second
	cout << "Data rate: " << fixed << setprecision(3) << rate << " MBPS\n";

    return 0;
}
