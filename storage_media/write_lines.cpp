#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sys/timeb.h>
#include "utils.h"
#include <stdio.h>
#include <string.h>

using namespace std;

/*
 * reads the input file line-by-line, converts each line to a record,
 * then write it to an output file.
 */
int main(int argc, char* argv[]) {

	if(argc != 2) {
        cerr << "Usage: write_lines <input filename>\n";
        exit(1);
	}


	char * in_filename = argv[1];

	char current_line[MAX_CHARS_PER_LINE];
	FILE *fp_read, *fp_write;

	if (!(fp_read = fopen(in_filename, "r"))){
		cerr << "Could not open file '" << in_filename << "' for reading\n";
		exit(1);
	}

	if (!(fp_write = fopen("lines.csv", "w"))){
		cerr << "Could not write to file lines.csv\n";
		exit(1);
	}

	// find the number of records
	// necessary because need to give a range for the random number
	fseek(fp_read, 0L, SEEK_END);
	long length = ftell(fp_read);

	// rewind inFile
	rewind(fp_read);

	struct timeb t_begin, t_end;
	long time_spent_ms = 0;


	// write lines just write each line. dont need to move into records
	ftime(&t_begin);
	while (fgets (current_line, MAX_CHARS_PER_LINE, fp_read)!=NULL){
		//current_line [strcspn (current_line, "\r\n")] = '\0';
		fprintf(fp_write, "%s", current_line);
	}
	ftime(&t_end);
	fclose(fp_read);

	// time elapsed in milliseconds
	time_spent_ms = (long) (1000 *(t_end.time - t_begin.time)
		  		  + (t_end.millitm - t_begin.millitm));

	float rate = (length/(float)time_spent_ms * 1000)/MB;
	//float rate = (total_records*sizeof(record)/(float)time_spent_ms * 1000)/MB;

	// result in MB per second
	cout << "Data rate: " << fixed << setprecision(3) << rate << " MBPS\n";

    return 0;
}
