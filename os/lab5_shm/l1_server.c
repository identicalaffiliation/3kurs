#include "shmem.h"

int main(void)
{
    Message *msgptr;
    key_t key;
    int shmid, semid;

    setvbuf(stdout, NULL, _IONBF, 0);

    if ((key = ftok(KEY_PATH, 'A')) < 0) { perror("Невозможно получить ключ"); exit(1); }

    if ((shmid = shmget(key, sizeof(Message), PERM | IPC_CREAT)) < 0) {
        perror("Невозможно создать область"); exit(1);
    }

    if ((msgptr = (Message *)shmat(shmid, NULL, 0)) == (void *)-1) {
        perror("Ошибка присоединения"); exit(1);
    }

    if ((semid = semget(key, 2, PERM | IPC_CREAT)) < 0) {
        perror("Невозможно создать семафор"); exit(1);
    }
    printf("Сервер: shmid = %d, semid = %d, адрес сегмента = %p\n", shmid, semid, (void *)msgptr);
    printf("Сервер: жду клиента...\n");

    if (semop(semid, &proc_wait[0], 1) < 0) { perror("Невозможно выполнить операцию"); exit(1); }

    if (semop(semid, &mem_lock[0], 2) < 0) { perror("Невозможно выполнить операцию"); exit(1); }

    printf("Сервер: в разделяемой памяти: %s", msgptr->buff);

    if (semop(semid, &mem_unlock[0], 1) < 0) { perror("mem_unlock"); exit(1); }
    if (semop(semid, &proc_start[0], 1) < 0) { perror("proc_start"); exit(1); }

    if (shmdt(msgptr) < 0) { perror("Ошибка отключения"); exit(1); }

    exit(0);
}
