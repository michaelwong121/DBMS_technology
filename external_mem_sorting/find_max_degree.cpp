#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sys/timeb.h>
#include "merge.h"
#include <stdio.h>
#include <string.h>

using namespace std;

/*
 * This program is used to find the max degree given a dat file
 */
int main(int argc, char* argv[]) {

	if(argc != 4) {
        cerr << "Usage: find_max_degree <input filename> <blocksize> <column id>\n";
        exit(1);
	}

	char * filename = argv[1];
	int blocksize = atoi(argv[2]);
	int records_per_block = blocksize / sizeof(record);
	int col_id = atoi(argv[3]);

	if (col_id != UID_ONE && col_id != UID_TWO){
		cerr<< "column id has to be 1 or 2.\n";
		exit(1);
	}

	FILE *fp_read;
	if (!(fp_read = fopen(filename, "rb"))){
		cerr << "Could not open file '" << filename << "' for reading\n";
		exit(1);
	}

	int this_uid = 0;          // uid1 of current person
	int this_uid_follows = 0;  // how many people this person is following
	int max_follow = 0;         // max
	long no_of_users = 0;

	// create block buffer
	record *buffer = new record[records_per_block];

	long in_buf = 0;
	while ((in_buf = fread((char *) buffer, sizeof(record), records_per_block, fp_read))){
		for (long i = 0; i < in_buf; ++i){
			int uid = (col_id == UID_ONE)? buffer[i].UID1 : buffer[i].UID2;
			// a new person
			if (uid != this_uid){
				max_follow = max(this_uid_follows, max_follow);
				this_uid = uid;
				this_uid_follows = 1;
				no_of_users++;
			}
			// same person
			else {
				++this_uid_follows;
			}
		}
	}
	max_follow = max(this_uid_follows, max_follow);

	delete(buffer);
	fclose(fp_read);

	cout<<"number of users: "<<no_of_users<<'\n';
	cout<<"max follow: "<<max_follow << "\n";

    return 0;
}





