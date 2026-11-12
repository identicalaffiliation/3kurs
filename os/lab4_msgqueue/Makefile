CC     = cc
CFLAGS = -Wall -Wextra -O2
PROGS  = l1_server l1_client l2_server l2_client l3_server_msg l3_client_msg

all: $(PROGS)

l1_%: l1_%.c mesg.h
	$(CC) $(CFLAGS) -o $@ $<

l2_%: l2_%.c l2_common.h mesg.h
	$(CC) $(CFLAGS) -o $@ $<

l3_%: l3_%.c l3_msg.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS)
