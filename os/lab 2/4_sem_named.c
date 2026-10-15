/*
 * 4_sem_named.c — именованный семафор POSIX для синхронизации процессов
 *
 * Родитель и потомок (fork) по очереди пишут в общий файл log.txt.
 * Именованный семафор "/lab_sem" (значение 1) обеспечивает взаимное
 * исключение между процессами. Показаны sem_open с O_CREAT|O_EXCL,
 * открытие существующего семафора, файл в /dev/shm, sem_close, sem_unlink.
 */
#include <semaphore.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#define SEM_NAME "/lab_sem"
#define LINES 5

static void writer(const char *who, sem_t *s)
{
    for (int i = 1; i <= LINES; i++) {
        sem_wait(s);                                   /* вход в критическую секцию */
        FILE *f = fopen("log.txt", "a");
        fprintf(f, "%s (pid %d): строка %d — начало", who, getpid(), i);
        fflush(f);
        usleep(10000);                                 /* «долгая» запись */
        fprintf(f, " ... конец\n");
        fclose(f);
        sem_post(s);                                   /* выход */
        usleep(5000);
    }
}

int main(void)
{
    sem_unlink(SEM_NAME);                              /* на случай остатков прошлого запуска */
    remove("log.txt");

    sem_t *s = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (s == SEM_FAILED) { perror("sem_open"); return 1; }
    printf("Семафор %s создан, начальное значение 1\n", SEM_NAME);

    /* повторное создание с O_EXCL должно завершиться ошибкой EEXIST */
    sem_t *s2 = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (s2 == SEM_FAILED)
        printf("Повторный sem_open с O_CREAT|O_EXCL: %s (errno=EEXIST)\n", strerror(errno));

#ifdef __linux__
    /* в Linux именованные семафоры видны как файлы /dev/shm/sem.<имя>;
       в macOS они живут только в ядре и в файловой системе не видны */
    printf("Файл семафора в /dev/shm: ");
    fflush(stdout);
    if (system("ls -l /dev/shm | grep lab_sem")) {}
#endif

    pid_t pid = fork();
    if (pid == 0) {
        /* потомок открывает существующий семафор по имени (oflag = 0) */
        sem_t *cs = sem_open(SEM_NAME, 0);
        writer("Потомок ", cs);
        sem_close(cs);
        _exit(0);
    }
    writer("Родитель", s);
    wait(NULL);

    sem_close(s);
    sem_unlink(SEM_NAME);                              /* удаление из системы */
    printf("\nСодержимое log.txt (каждая строка цельная — записи не перемешаны):\n");
    fflush(stdout);
    if (system("cat log.txt")) {}
    /* проверка удаления: открыть семафор без O_CREAT больше нельзя */
    if (sem_open(SEM_NAME, 0) == SEM_FAILED)
        printf("\nПосле sem_unlink: sem_open(\"%s\", 0) -> %s — семафор удалён\n",
               SEM_NAME, strerror(errno));
    return 0;
}
