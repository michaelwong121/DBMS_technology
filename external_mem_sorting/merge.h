#ifndef MERGE_H
#define MERGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;

#define MAX_PATH_LENGTH 1024

//column id
#define UID_ONE 1
#define UID_TWO 2


//return values of all functions
#define SUCCESS 0
#define FAILURE 1
#define EMPTY 2

typedef struct record {
	int UID1;
	int UID2;
} record;

typedef struct HeapElement {
	int UID1;
	int UID2;
	int run_id;
} HeapElement;

// total merge wrapper
int ext_merge_sort (char *in_file, char *out_file, long total_mem, long blocksize, short col_id);

//record-keeping struct, to pass around to all small functions
//has to be initialized before merge starts
class MergeManager {

	short col_id; // used to decide if we are sorting by UID1 or UID2

	// heap related variables
	HeapElement *heap;  //keeps 1 from each buffer in top-down order - smallest on top (according to compare function)
	int current_heap_size;
	int heap_capacity;  //corresponds to the total number of runs (input buffers)

	// input buffer related variables
	char input_prefix [MAX_PATH_LENGTH] ; //stores the prefix of a path to each run - to concatenate with run id and to read the file
	FILE *inputFP; //stays closed, opens each time we need to reupload some amount of data from disk runs
	int *input_file_numbers;  //we need to know the run id to read from the corresponding run	
	record **input_buffers; //array of buffers to buffer part of runs
	int input_buffer_capacity; //how many elements max can each input buffer hold
	int *current_input_file_positions; //current position in each sorted run, can use -1 if the run is complete
	int *current_input_buffer_positions; //position in current input buffer
	int *total_input_buffer_elements;  //number of actual elements currently in input buffer - can be less than max capacity

	// output related variables
	char output_file_name [MAX_PATH_LENGTH]; //stores name of the file to which to write the final output
	FILE *outputFP; //flushes output from output buffer
	record *output_buffer; //buffer to store output elements until they are flushed to disk
	int current_output_buffer_position;  //where to add element in the output buffer
	int output_buffer_capacity; //how many elements max can it hold

public:

	MergeManager();
	~MergeManager(); // use destructor to clean up instead

	//1. main loop
	int merge_runs ();

	int init_merge (int, long, long, char*, short);

	int flush_output_buffer ();

	int get_top_heap_element (HeapElement * result);

	int insert_into_heap (int run_id, record &input);

	int get_next_input_element(int run_id, record &result);

	int refill_buffer (int run_id);

	int compare_heap_elements (HeapElement &a, HeapElement &b);

	// utility functions for checking buffer and heap
	void print_input_buf (int run_id);
	void print_all_input_buf ();
	void print_heap ();
};


// other functions
int compare_uid1 (const void *a, const void *b);
int compare_uid2 (const void *a, const void *b);


#endif
