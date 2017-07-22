#ifndef JOIN_H
#define JOIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merge.h"
#include <iostream>

using namespace std;

#define MAX_PATH_LENGTH 1024

//column id
#define UID_ONE 1
#define UID_TWO 2

// buffer type
#define INPUT  0
#define OUTPUT 1

//return values of all functions
#define SUCCESS 0
#define FAILURE 1
#define EMPTY 2

class buffer {
	short type; // input buffer or output buffer
	FILE *fp;

	int capacity; // max number of records it can fit in the buffer
	int in_buf; // number of records in the buffer. It should equal capacity most of the time
				// except the last time.
	int current_pos;

	record *buf;

public:
	buffer(short type, int capacity);
	~buffer();

	int init_fp(char* file);
	void reset_buf();
	int read_from_buffer(record &r); // read 1 record from buf. Update from disk if necessary
	void print_buf();

};

typedef struct celeb{
	int UID;
	int in_deg;
	int out_deg;
} celeb;

// this is a sorted list of celebrities
class top_celeb_list{
	celeb *list;
	int num_in_list;

public:
	top_celeb_list(int capacity);
	~top_celeb_list();

	void replace_celeb(celeb c);
	void print_list();
};

int compare_fame(const void *a, const void *b);

// binary search related funcitons
int bin_search(record *buf, int start, int end, int uid2);
int scan_for_record(record *buf, int capacity, int index, record s);

#endif
