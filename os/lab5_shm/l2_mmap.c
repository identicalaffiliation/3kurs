/*
 * l2_mmap.c — Уровень 2 (A): разбор скелета (mmap + именованный семафор POSIX)
 *
 * Что происходит:
 *  1. Открывается файл filedata, первый его байт отображается в память
 *     (mmap, MAP_SHARED): запись в ptr[0] — это запись в файл, и она
 *     видна всем процессам, отобразившим этот же файл.
 *  2. Дескриптор файла можно закрыть — отображение остаётся.
 *  3. Создаётся именованный семафор со значением 1 и сразу удаляется
 *     по имени (sem_unlink): процессы, уже открывшие его, работают дальше.
 *  4. fork(): потомок наследует и отображение, и открытый семафор.
 *  5. Родитель и потомок по 10 раз под семафором печатают символ
 *     и увеличивают его (*ptr)++. Символы идут без повторов: a, b, c, ...
 *
 * Исправления скелета:
 *  - путь "home/tvk/IPC/filedata" без начального '/' и чужой — здесь
 *    файл filedata в текущей папке; если его нет, он создаётся с байтом 'a';
 *  - файл был открыт O_RDONLY, а отображается с PROT_WRITE и MAP_SHARED —
 *    mmap вернёт EACCES. Нужен O_RDWR;
 *  - результат mmap не проверялся (ошибка — MAP_FAILED);
 *  - px_ipc_name — вспомогательная функция из книги Стивенса, в системе
 *    её нет. Имя семафора — просто "/mysem";
 *  - sem_open с O_CREAT требует 4 аргумента (права и начальное значение).
 *    Начальное значение должно быть 1: при 0 оба процесса вечно ждут;
 *  - родитель не ждал потомка (wait).
 *
 * Запуск: ./l2_mmap [число_итераций]
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define FILE_NAME "filedata"
#define SEM_NAME  "/mysem"

int main(int argc, char **argv)
{
    int fd, i, nloop = argc > 1 ? atoi(argv[1]) : 10;
    char *ptr;
    sem_t *mutex;

    if ((fd = open(FILE_NAME, O_RDWR | O_CREAT, 0644)) < 0) { perror("Can not open file"); exit(1); }
    if (lseek(fd, 0, SEEK_END) == 0) {               /* пустой файл: запишем первый байт */
        if (write(fd, "a", 1) != 1) { perror("write"); exit(1); }
    }
    printf("File is opened!\n");

    ptr = mmap(NULL, sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); exit(1); }
    close(fd);                                       /* отображение остаётся */
    printf("Отображён 1 байт файла по адресу %p, сейчас там '%c'\n", (void *)ptr, *ptr);

    sem_unlink(SEM_NAME);                            /* на случай остатков */
    mutex = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (mutex == SEM_FAILED) { perror("sem_open"); exit(1); }
    sem_unlink(SEM_NAME);                            /* имя больше не нужно */
    setbuf(stdout, NULL);

    if (fork() == 0) {
        for (i = 0; i < nloop; i++) {
            sem_wait(mutex);
            printf("Child:  %c\n", (*ptr)++);
            sem_post(mutex);
        }
        exit(0);
    }
    for (i = 0; i < nloop; i++) {
        sem_wait(mutex);
        printf("Parent: %c\n", (*ptr)++);
        sem_post(mutex);
    }
    wait(NULL);
    printf("Итог: в первом байте файла %s теперь '%c' — изменения через память попали в файл\n",
           FILE_NAME, *ptr);
    munmap(ptr, sizeof(char));
    sem_close(mutex);
    exit(0);
}
