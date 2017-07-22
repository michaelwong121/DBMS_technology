CC = g++
CFLAGS = -Wall
CFLAGS += -D_LARGEFILE_SOURCE
CFLAGS += -fno-exceptions
CFLAGS += -finline-functions
CFLAGS += -funroll-loops
CFLAGS += -D_FILE_OFFSET_BITS=64

FILES = disk_sort
FILES += read_test
FILES += distribution
FILES += find_max_degree
FILES += write_blocks_seq


all: $(FILES)
		
disk_sort: disk_sort.cpp merge.h merge_external.cpp
	$(CC) $(CFLAGS) -o $@ $^
	
read_test: read_test.cpp merge.h
	$(CC) $(CFLAGS) -o $@ $^
	
distribution: distribution.cpp merge.h
	$(CC) $(CFLAGS) -o $@ $^
	
find_max_degree: find_max_degree.cpp merge.h
	$(CC) $(CFLAGS) -o $@ $^

write_blocks_seq: write_blocks_seq.cpp merge.h
	$(CC) $(CFLAGS) -o $@ $^

clean : $(FILES)
	rm -f *.o $^ *~
