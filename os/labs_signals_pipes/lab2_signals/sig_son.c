/*
 * sig_son.c — Уровень 2 (A), процесс-сын
 *   - получает идентификатор родителя (getppid);
 *   - посылает отцу SIGUSR1;
 *   - проверяет, удачно ли отправлен сигнал.
 */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

int main(void)
{
    pid_t father = getppid();
    printf("sig_son is starting! pid=%d ppid=%d\n", getpid(), father);
    fflush(stdout);

    if (kill(father, SIGUSR1) == -1) {
        perror("sig_son: Send signal with Error");
        return 1;
    }
    printf("sig_son: Send signal SIGUSR1 to father (pid %d) successfully!\n", father);
    return 0;
}
