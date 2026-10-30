#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PSFILE "ps.txt"

static void son3_handler(int sig)
{
    char msg[96];
    int n = snprintf(msg, sizeof msg,
                     "son3 (pid %d): перехватил сигнал %d (SIGUSR1) и обработал сам\n",
                     getpid(), sig);
    if (write(STDOUT_FILENO, msg, n) < 0) {}       
}

static void ps_snapshot(const char *title, pid_t *sons)
{
    char cmd[256];
#ifdef __APPLE__
    snprintf(cmd, sizeof cmd,
             "echo '===== %s =====' >> %s; "
             "ps -o pid,ppid,stat,command -p %d,%d,%d,%d >> %s",
#else
    snprintf(cmd, sizeof cmd,
             "echo '===== %s =====' >> %s; "
             "ps s -p %d,%d,%d,%d >> %s",
#endif
             title, PSFILE, getpid(), sons[0], sons[1], sons[2], PSFILE);
    if (system(cmd)) {}
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);      
    remove(PSFILE);

    pid_t sons[3];
    printf("father: pid = %d\n", getpid());

    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); exit(1); }
        if (pid == 0) {
            switch (i) {
            case 0: signal(SIGUSR1, SIG_DFL);      break;  
            case 1: signal(SIGUSR1, SIG_IGN);      break; 
            case 2: signal(SIGUSR1, son3_handler); break; 
            }
            printf("son%d: pid = %d, ppid = %d, SIGUSR1 -> %s\n", i + 1, getpid(), getppid(),
                   i == 0 ? "SIG_DFL" : i == 1 ? "SIG_IGN" : "свой обработчик");
            for (;;) pause();               
        }
        sons[i] = pid;
    }

    sleep(1);                     
    ps_snapshot("ДО посылки SIGUSR1", sons);

    for (int i = 0; i < 3; i++) {
        if (kill(sons[i], SIGUSR1) == 0)
            printf("father: SIGUSR1 отправлен son%d (pid %d)\n", i + 1, sons[i]);
        else
            perror("kill");
    }

    sleep(1);
    ps_snapshot("ПОСЛЕ посылки SIGUSR1", sons);

    kill(sons[1], SIGTERM);
    kill(sons[2], SIGTERM);

    for (int i = 0; i < 3; i++) {
        int st;
        waitpid(sons[i], &st, 0);
        if (WIFSIGNALED(st))
            printf("father: son%d завершён сигналом %d (%s)\n",
                   i + 1, WTERMSIG(st), strsignal(WTERMSIG(st)));
        else
            printf("father: son%d завершился с кодом %d\n", i + 1, WEXITSTATUS(st));
    }
    printf("father: таблица процессов записана в %s\n", PSFILE);
    return 0;
}
