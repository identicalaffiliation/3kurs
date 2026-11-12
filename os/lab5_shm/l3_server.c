/*
 * l3_server.c — Уровень 3 (A): сервер
 *
 * Создаёт сегмент разделяемой памяти и группу семафоров, затем в цикле
 * обслуживает клиентов: читает из памяти имя файла и кладёт туда
 * содержимое файла порциями (или текст ошибки).
 * Остановка: Ctrl+C или kill -INT <pid> — объекты IPC удаляются.
 *
 * Запуск: ./l3_server &   затем   echo /путь/к/файлу | ./l3_client
 */
#include <signal.h>
#include "l3_shm.h"

static int shmid = -1, semid = -1;
static ShmMsg *box;

static void cleanup(int sig)
{
    (void)sig;
    if (box) shmdt(box);
    if (shmid >= 0) shmctl(shmid, IPC_RMID, NULL);
    if (semid >= 0) semctl(semid, 0, IPC_RMID);
    const char m[] = "\nСервер: SIGINT — память и семафоры удалены\n";
    if (write(STDOUT_FILENO, m, sizeof m - 1) < 0) {}
    _exit(0);
}

/* Создать объект заново: если остался от прошлого запуска — удалить */
static int create_shm(key_t key)
{
    int id = shmget(key, sizeof(ShmMsg), PERM | IPC_CREAT | IPC_EXCL);
    if (id < 0 && errno == EEXIST) {
        shmctl(shmget(key, 0, 0), IPC_RMID, NULL);
        id = shmget(key, sizeof(ShmMsg), PERM | IPC_CREAT | IPC_EXCL);
    }
    return id;
}
static int create_sem(key_t key)
{
    int id = semget(key, NSEMS, PERM | IPC_CREAT | IPC_EXCL);
    if (id < 0 && errno == EEXIST) {
        semctl(semget(key, 0, 0), 0, IPC_RMID);
        id = semget(key, NSEMS, PERM | IPC_CREAT | IPC_EXCL);
    }
    return id;
}

/* Ждать подтверждения от клиента. Если клиент за это время умер
   (например, его убили Ctrl+C), обычный P(SEM_ACK) ждал бы вечно.
   Поэтому семафор опрашивается без блокировки (IPC_NOWAIT), а между
   попытками проверяется, жив ли клиент: kill(pid, 0) шлёт «пустой» сигнал. */
static int wait_ack(void)
{
    for (;;) {
        if (sem_op(semid, SEM_ACK, -1, IPC_NOWAIT) == 0) return 0;
        if (errno != EAGAIN) return -1;
        if (kill(box->client, 0) < 0 && errno == ESRCH) return -1;
        usleep(1000);
    }
}

static void serve_one(int n)
{
    char name[CHUNK + 1];
    int fd;
    ssize_t r;
    int parts = 0;
    long total = 0;

    memcpy(name, box->data, box->len);
    name[box->len] = '\0';
    printf("Сервер: [%d] запрошен файл \"%s\"\n", n, name);

    if ((fd = open(name, O_RDONLY)) < 0) {
        box->type = T_ERROR;
        box->len = snprintf(box->data, CHUNK, "Ошибка: невозможно открыть %s: %s\n",
                            name, strerror(errno));
        V(semid, SEM_RESP);
        wait_ack();
        printf("Сервер: [%d] отправлено сообщение об ошибке\n", n);
        return;
    }
    while ((r = read(fd, box->data, CHUNK)) > 0) {
        box->type = T_DATA;
        box->len = (int)r;
        V(semid, SEM_RESP);             /* порция готова */
        if (wait_ack() < 0) {           /* ждём, пока клиент заберёт */
            printf("Сервер: [%d] клиент pid %d пропал, передача прервана\n", n, (int)box->client);
            close(fd);
            semctl(semid, SEM_RESP, SETVAL, (union semun){ .val = 0 });
            return;
        }
        parts++;
        total += r;
    }
    close(fd);
    box->type = T_END;
    box->len = 0;
    V(semid, SEM_RESP);
    wait_ack();
    printf("Сервер: [%d] отправлено %ld байт порциями по %d: %d порц.\n", n, total, CHUNK, parts);
}

int main(void)
{
    key_t key;
    union semun arg;
    unsigned short init[NSEMS] = { 1, 0, 0, 0 };
    int n = 0;

    setvbuf(stdout, NULL, _IONBF, 0);
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    if ((key = ftok(KEY_PATH, KEY_ID)) < 0) { perror("ftok"); exit(1); }
    if ((shmid = create_shm(key)) < 0) { perror("shmget"); exit(1); }
    if ((box = (ShmMsg *)shmat(shmid, NULL, 0)) == (void *)-1) { perror("shmat"); cleanup(0); }
    if ((semid = create_sem(key)) < 0) { perror("semget"); cleanup(0); }
    arg.array = init;
    if (semctl(semid, 0, SETALL, arg) < 0) { perror("semctl SETALL"); cleanup(0); }

    printf("Сервер: pid = %d, shmid = %d (%zu байт), semid = %d\n",
           getpid(), shmid, sizeof(ShmMsg), semid);

    for (;;) {
        if (P(semid, SEM_REQ) < 0) { perror("semop"); cleanup(0); }
        serve_one(++n);
    }
}
