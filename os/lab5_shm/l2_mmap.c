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
    if (lseek(fd, 0, SEEK_END) == 0) {            
        if (write(fd, "a", 1) != 1) { perror("write"); exit(1); }
    }
    printf("File is opened!\n");

    ptr = mmap(NULL, sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); exit(1); }
    close(fd);                                      
    printf("Отображён 1 байт файла по адресу %p, сейчас там '%c'\n", (void *)ptr, *ptr);

    sem_unlink(SEM_NAME);                          
    mutex = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (mutex == SEM_FAILED) { perror("sem_open"); exit(1); }
    sem_unlink(SEM_NAME);                          
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
