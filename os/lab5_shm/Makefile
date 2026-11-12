CC     = cc
CFLAGS = -Wall -Wextra -O2
PROGS  = l1_server l1_client l2_mmap l3_server l3_client
# именованные семафоры: в Linux нужен -pthread, в macOS всё в libSystem
LDLIBS = $(if $(filter Linux,$(shell uname -s)),-pthread,)

all: $(PROGS)

l1_%: l1_%.c shmem.h
	$(CC) $(CFLAGS) -o $@ $<

l2_mmap: l2_mmap.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

l3_%: l3_%.c l3_shm.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS) filedata
