#include "merge.h"

/* This program is used to check if the file is sorted in the correct order*/
int main (int argc, char **argv) {
	
	//process and validate command-line arguments
	if(argc != 3) {
		cerr << "Usage: read_test <input filename> <column id>\n";
		exit(FAILURE);
	}

	char * filename = argv[1];
	int col_id = atoi(argv[2]);

	if (col_id != UID_ONE && col_id != UID_TWO){
		cerr<< "column id has to be 1 or 2.\n";
		exit(1);
	}

	FILE *fp_read;
	if (!(fp_read = fopen(filename, "rb"))){
		cerr << "Could not open file '" << filename << "' for reading\n";
		exit(FAILURE);
	}

	// find the number of records
	fseek(fp_read, 0L, SEEK_END);
	long length = ftell(fp_read);
	long total_records = length / sizeof(record); // total number of records
	rewind(fp_read);// rewind inFile


	record *ram_buf = new record[total_records]; // buffer that represents ram
	int result = fread(ram_buf, sizeof(record), total_records, fp_read);
	if (result != total_records){
		cerr << "Reading error!\n";
	}
	fclose(fp_read);

	int prev_uid = 0;
	for (int i = 0; i < total_records; ++i){
		if (col_id == UID_ONE){
			if (prev_uid > ram_buf[i].UID1){
				cout<<"Problem at: "<< i<< '\n';
				exit(1);
			}
			prev_uid = ram_buf[i].UID1;
		}
		else if (col_id == UID_TWO){
			if (prev_uid > ram_buf[i].UID2){
				cout<<"Problem at: "<< i<< '\n';
				exit(1);
			}
			prev_uid = ram_buf[i].UID2;
		}
		//cout<< "Record " << i+1<<", uid: "<<ram_buf[i].UID1<<" "<<ram_buf[i].UID2<<'\n';
	}

	delete(ram_buf);

	return 0;
}
