CC = g++
CFLAGS = -O3 -Wall 
CFLAGS += -D_LARGEFILE_SOURCE
CFLAGS += -fno-exceptions
CFLAGS += -finline-functions
CFLAGS += -funroll-loops
CFLAGS += -D_FILE_OFFSET_BITS=64

# add files that doesn't need utils here
FILES = write_blocks_seq
FILES += write_lines
FILES += read_blocks_seq
FILES += read_ram_seq 
FILES += read_blocks_rand
FILES += read_ram_rand 
FILES += write_blocks_rand
FILES += write_ram_rand

all: $(FILES)

write_blocks_seq: write_blocks_seq.o
	$(CC) $(CFLAGS) -o $@ $^

write_lines: write_lines.o
	$(CC) $(CFLAGS) -o $@ $^
		
read_blocks_seq: read_blocks_seq.o
	$(CC) $(CFLAGS) -o $@ $^
	
read_ram_seq: read_ram_seq.o
	$(CC) $(CFLAGS) -o $@ $^

read_blocks_rand: read_blocks_rand.o
	$(CC) $(CFLAGS) -o $@ $^
	
read_ram_rand: read_ram_rand.o
	$(CC) $(CFLAGS) -o $@ $^

write_blocks_rand: write_blocks_rand.o
	$(CC) $(CFLAGS) -o $@ $^
	
write_ram_rand: write_ram_rand.o
	$(CC) $(CFLAGS) -o $@ $^

%.o : %.cpp utils.h
	$(CC) $(CFLAGS) -g -c $<

clean : $(FILES)
	rm -f *.o $^ *~
