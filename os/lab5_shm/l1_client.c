#include "shmem.h"

int main(void)
{
    Message *msgptr;
    key_t key;
    int shmid, semid;

    if ((key = ftok(KEY_PATH, 'A')) < 0) { perror("Невозможно получить ключ"); exit(1); }

    if ((shmid = shmget(key, sizeof(Message), 0)) < 0) { perror("Ошибка доступа (сервер запущен?)"); exit(1); }

    if ((msgptr = (Message *)shmat(shmid, NULL, 0)) == (void *)-1) { perror("Ошибка присоединения"); exit(1); }

    if ((semid = semget(key, 2, PERM)) < 0) { perror("Ошибка доступа"); exit(1); }

    if (semop(semid, &mem_lock[0], 2) < 0) { perror("Невозможно выполнить операцию"); exit(1); }

    if (semop(semid, &proc_start[0], 1) < 0) { perror("Невозможно выполнить операцию"); exit(1); }

    snprintf(msgptr->buff, MAXBUFF, "Good luck!\n");
    printf("Клиент: записал в разделяемую память: %s", msgptr->buff);

    if (semop(semid, &mem_unlock[0], 1) < 0) { perror("Невозможно выполнить операцию"); exit(1); }

    if (semop(semid, &proc_wait[0], 1) < 0) { perror("Невозможно выполнить операцию"); exit(1); }

    if (shmdt(msgptr) < 0) { perror("Ошибка отключения"); exit(1); }

    if (shmctl(shmid, IPC_RMID, NULL) < 0) { perror("Невозможно удалить область"); exit(1); }
    if (semctl(semid, 0, IPC_RMID) < 0) { perror("Невозможно удалить семафор"); exit(1); }
    printf("Клиент: разделяемая память и семафоры удалены\n");
    exit(0);
}
