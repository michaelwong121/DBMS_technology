CC = g++
CFLAGS = -O3 -Wall 
CFLAGS += -D_LARGEFILE_SOURCE
CFLAGS += -fno-exceptions
CFLAGS += -finline-functions
CFLAGS += -funroll-loops
CFLAGS += -D_FILE_OFFSET_BITS=64

# add files that doesn't need utils here
FILES = find_true_friend
FILES += top_celeb
FILES += read_test

all: $(FILES)
		
find_true_friend: find_true_friend.o join_external.o merge_external.o
	$(CC) $(CFLAGS) -o $@ $^
	
top_celeb: top_celeb.o join_external.o merge_external.o
	$(CC) $(CFLAGS) -o $@ $^
	
read_test : read_test.o
	$(CC) $(CFLAGS) -o $@ $^
	
%.o : %.cpp join.h merge.h
	$(CC) $(CFLAGS) -g -c $<

clean : $(FILES)
	rm -f *.o $^ *~
