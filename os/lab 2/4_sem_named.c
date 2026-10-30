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
        sem_wait(s);                                  
        FILE *f = fopen("log.txt", "a");
        fprintf(f, "%s (pid %d): строка %d — начало", who, getpid(), i);
        fflush(f);
        usleep(10000);                               
        fprintf(f, " ... конец\n");
        fclose(f);
        sem_post(s);                                 
        usleep(5000);
    }
}

int main(void)
{
    sem_unlink(SEM_NAME);                            
    remove("log.txt");

    sem_t *s = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (s == SEM_FAILED) { perror("sem_open"); return 1; }
    printf("Семафор %s создан, начальное значение 1\n", SEM_NAME);

    sem_t *s2 = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 1);
    if (s2 == SEM_FAILED)
        printf("Повторный sem_open с O_CREAT|O_EXCL: %s (errno=EEXIST)\n", strerror(errno));

    pid_t pid = fork();
    if (pid == 0) {
        sem_t *cs = sem_open(SEM_NAME, 0);
        writer("Потомок ", cs);
        sem_close(cs);
        _exit(0);
    }
    writer("Родитель", s);
    wait(NULL);

    sem_close(s);
    sem_unlink(SEM_NAME);                             
    printf("\nСодержимое log.txt (каждая строка цельная — записи не перемешаны):\n");
    fflush(stdout);
    if (system("cat log.txt")) {}
    if (sem_open(SEM_NAME, 0) == SEM_FAILED)
        printf("\nПосле sem_unlink: sem_open(\"%s\", 0) -> %s — семафор удалён\n",
               SEM_NAME, strerror(errno));
    return 0;
}
