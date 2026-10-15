CC     = cc
CFLAGS = -Wall -Wextra -O2 -pthread
PROGS  = 1_mutex 2_spinlock 3_sem_unnamed 4_sem_named
# librt нужна только в Linux; в macOS её нет (всё уже в libSystem)
LDLIBS = $(if $(filter Linux,$(shell uname -s)),-lrt,)

all: $(PROGS)

%: %.c spin_compat.h sem_compat.h
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

run: all
	@for p in $(PROGS); do echo "########## ./$$p ##########"; ./$$p; echo; done

clean:
	rm -f $(PROGS) log.txt
