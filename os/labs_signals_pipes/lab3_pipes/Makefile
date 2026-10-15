CC     = gcc
CFLAGS = -Wall -O2
PROGS  = pipe_simple pipe_cs hello_server hello_client \
         server_fifo1 client_fifo1 server_fifomsg client_fifomsg

all: $(PROGS)

%: %.c common.h mesg.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS) output.txt /tmp/fifo.1 /tmp/fifo.2 /tmp/fifo.hello
