/*
 * father.c — Уровень 1 (A)
 *
 * 1) Родитель порождает три сына: son1, son2, son3.
 * 2) Родитель посылает SIGUSR1 каждому сыну.
 * 3) Реакция сыновей на SIGUSR1:
 *      son1 — действие по умолчанию (завершение процесса),
 *      son2 — игнорирует сигнал,
 *      son3 — перехватывает и обрабатывает сам.
 * Таблица процессов до и после посылки сигналов пишется в файл ps.txt
 * через system("ps ... >> file").
 *
 * Примечание: в современном procps ключ "-s" требует список сессий,
 * поэтому используется BSD-формат "ps s": он показывает маски
 * PENDING / BLOCKED / IGNORED / CAUGHT. SIGUSR1 = 10 -> бит 0x200.
 */
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
    if (write(STDOUT_FILENO, msg, n) < 0) {}            /* write безопасен в обработчике */
}

static void ps_snapshot(const char *title, pid_t *sons)
{
    char cmd[256];
#ifdef __APPLE__
    /* в macOS у ps нет столбцов IGNORED/CAUGHT, показываем состояние (STAT) */
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
    setvbuf(stdout, NULL, _IONBF, 0);        /* без буфера: вывод не дублируется после fork */
    remove(PSFILE);

    pid_t sons[3];
    printf("father: pid = %d\n", getpid());

    /* --- шаг 1: порождаем сыновей --- */
    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); exit(1); }
        if (pid == 0) {
            /* --- шаг 3: каждый сын задаёт свою реакцию на SIGUSR1 --- */
            switch (i) {
            case 0: signal(SIGUSR1, SIG_DFL);      break;  /* по умолчанию */
            case 1: signal(SIGUSR1, SIG_IGN);      break;  /* игнорировать */
            case 2: signal(SIGUSR1, son3_handler); break;  /* перехватить */
            }
            printf("son%d: pid = %d, ppid = %d, SIGUSR1 -> %s\n", i + 1, getpid(), getppid(),
                   i == 0 ? "SIG_DFL" : i == 1 ? "SIG_IGN" : "свой обработчик");
            for (;;) pause();                /* ждём сигналов */
        }
        sons[i] = pid;
    }

    sleep(1);                                /* даём сыновьям установить диспозицию */
    ps_snapshot("ДО посылки SIGUSR1", sons);

    /* --- шаг 2: посылаем SIGUSR1 всем сыновьям --- */
    for (int i = 0; i < 3; i++) {
        if (kill(sons[i], SIGUSR1) == 0)
            printf("father: SIGUSR1 отправлен son%d (pid %d)\n", i + 1, sons[i]);
        else
            perror("kill");
    }

    sleep(1);
    ps_snapshot("ПОСЛЕ посылки SIGUSR1", sons);

    /* son1 уже завершился (и пока висит зомби), son2 и son3 живы — завершаем их */
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
