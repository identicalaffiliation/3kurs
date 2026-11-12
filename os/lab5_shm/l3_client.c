/*
 * l3_client.c — Уровень 3 (A): клиент
 *
 * Читает полное имя файла из stdin, кладёт его в разделяемую память,
 * затем выводит в stdout всё, что сервер положит в память:
 * содержимое файла или сообщение об ошибке.
 */
#include <signal.h>
#include "l3_shm.h"

int main(void)
{
    key_t key;
    int shmid, semid;
    ShmMsg *box;
    char name[CHUNK];
    size_t len;
    int done = 0;

    /* если вывод закрыли (например, | head), не умирать от SIGPIPE,
       а доиграть протокол с сервером до конца */
    signal(SIGPIPE, SIG_IGN);

    if ((key = ftok(KEY_PATH, KEY_ID)) < 0) { perror("ftok"); exit(1); }
    if ((shmid = shmget(key, sizeof(ShmMsg), 0)) < 0) { perror("Клиент: shmget (сервер запущен?)"); exit(1); }
    if ((semid = semget(key, NSEMS, 0)) < 0) { perror("Клиент: semget"); exit(1); }
    if ((box = (ShmMsg *)shmat(shmid, NULL, 0)) == (void *)-1) { perror("Клиент: shmat"); exit(1); }

    fprintf(stderr, "Клиент: введите полное имя файла: ");
    if (fgets(name, sizeof name, stdin) == NULL) exit(1);
    len = strlen(name);
    if (len > 0 && name[len - 1] == '\n') len--;
    fprintf(stderr, "%.*s\n", (int)len, name);

    /* SEM_UNDO: если клиент упадёт, ядро само вернёт серверу «свободно» */
    if (sem_op(semid, SEM_FREE, -1, SEM_UNDO) < 0) { perror("semop FREE"); exit(1); }

    box->client = getpid();
    box->type = T_REQUEST;
    box->len = (int)len;
    memcpy(box->data, name, len);
    V(semid, SEM_REQ);

    while (!done) {
        if (P(semid, SEM_RESP) < 0) { perror("semop RESP"); exit(1); }
        switch (box->type) {
        case T_DATA:
            if (write(STDOUT_FILENO, box->data, box->len) != box->len && errno != EPIPE) perror("write");
            break;
        case T_ERROR:
            if (write(STDOUT_FILENO, box->data, box->len) != box->len) perror("write");
            done = 1;
            break;
        default:                          /* T_END */
            done = 1;
        }
        V(semid, SEM_ACK);
    }

    sem_op(semid, SEM_FREE, +1, SEM_UNDO);
    shmdt(box);
    return 0;
}
