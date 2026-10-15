CC     = cc
CFLAGS = -Wall -O2
PROGS  = father sig_father sig_son thread_sig sigact

all: $(PROGS)

thread_sig: thread_sig.c
	$(CC) $(CFLAGS) -pthread -o $@ $<

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS) ps.txt
