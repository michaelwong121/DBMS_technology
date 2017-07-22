#include <cstdlib>
#include <iomanip>
#include <sys/timeb.h>
#include "merge.h"

using namespace std;
#define MAX_CHARS_PER_LINE 1024

/*
 * reads the input file line-by-line, converts each line to a record,
 * fills-in the output page buffer (the size of the buffer corresponds
 * to the block size, provided as a program parameter), and - when the
 * buffer is full - appends the full page of serialized records to a binary file.
 */
int main(int argc, char* argv[]) {

	if(argc != 4) {
        cerr << "Usage: write_blocks_seq <input filename> <output filename> <blocksize>\n";
        exit(1);
	}

	char * in_filename = argv[1];
	char * out_filename = argv[2];
	int blocksize = atoi(argv[3]);
	int records_per_block = blocksize / sizeof(record);

	char current_line[MAX_CHARS_PER_LINE];
	char *endptr;
	FILE *fp_read, *fp_write;

	if (!(fp_read = fopen(in_filename, "r"))){
		cerr << "Could not open file '" << in_filename << "' for reading\n";
		exit(1);
	}

	if (!(fp_write = fopen(out_filename, "wb"))){
		cerr << "Could not write to file " << out_filename<< "\n";
		exit(1);
	}

	long total_records = 0;

	int in_buf = 0; // number of records in the buffer
	record *buffer = new record[records_per_block];

	while( fgets (current_line, MAX_CHARS_PER_LINE, fp_read)!=NULL ) {
		current_line [strcspn (current_line, "\r\n")] = '\0'; //remove end-of-line characters
		record &r = buffer[in_buf];
		r.UID1 = strtol(current_line, &endptr, 10);
		r.UID2 = strtol(endptr+1, NULL, 10);
		total_records++;
		in_buf++;
		if (in_buf == records_per_block){
			fwrite(buffer, sizeof(record), in_buf, fp_write);
			in_buf = 0; // reset pointer to position 0
		}
	}
	fwrite(buffer, sizeof(record), in_buf, fp_write);

	fclose(fp_read);
	fclose(fp_write);

	delete (buffer);

    return 0;
}
