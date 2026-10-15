/*
 * sigact.c — Уровни 1, 2, 3 (B), надёжные сигналы (sigaction)
 *
 * Обработчик SIGUSR1/SIGUSR2 «засыпает» на SLEEP секунд (по заданию 60).
 * В sa_mask добавлен SIGINT, поэтому пока работает обработчик,
 * SIGINT ЗАБЛОКИРОВАН: он не теряется, а остаётся в ожидании (pending)
 * и доставляется сразу после выхода из обработчика.
 *
 * Запуск:
 *   ./sigact [сек] [-k] &
 *     сек — длительность сна в обработчике (по умолчанию 60)
 *     -k  — вариант 2: обработчик сам посылает себе SIGINT
 *           через kill(getpid(), SIGINT)
 *   Затем с терминала: kill -s USR1 <PID>  (или USR2), потом kill -s INT <PID>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

static int sleep_sec = 60;
static int self_int  = 0;

static void sig_handler(int sig)
{
    printf("Signal %d (%s) is coming — обработчик засыпает на %d с, SIGINT заблокирован\n",
           sig, strsignal(sig), sleep_sec);
    if (self_int) {
        printf("  обработчик: kill(getpid(), SIGINT) — SIGINT отложен до выхода из обработчика\n");
        kill(getpid(), SIGINT);
    }
    sleep(sleep_sec);
    printf("Signal %d: обработчик завершён, маска восстановлена\n", sig);
}

/* надёжная установка обработчика (функция mysig из методички):
   возвращает предыдущий обработчик или SIG_ERR */
typedef void (*sighandler_fn)(int);
static sighandler_fn mysig(int sig, sighandler_fn handler)
{
    struct sigaction act, oldact;
    act.sa_handler = handler;              /* обработчик сигнала sig */
    sigemptyset(&act.sa_mask);             /* обнуление маски */
    sigaddset(&act.sa_mask, SIGINT);       /* блокировка SIGINT на время обработчика */
    act.sa_flags = 0;
    if (sigaction(sig, &act, &oldact) < 0)
        return SIG_ERR;
    return oldact.sa_handler;
}

static void set_action(int sig, struct sigaction *new_action, struct sigaction *old_action)
{
    new_action->sa_handler = sig_handler;
    sigemptyset(&new_action->sa_mask);
    sigaddset(&new_action->sa_mask, SIGINT);
    new_action->sa_flags = 0;
    if (sigaction(sig, new_action, old_action) < 0) {
        printf("Action is not setted\n");
        exit(1);
    }
    printf("Action for signal %d is setted\n", sig);
}

static void restore_action(int sig, struct sigaction *old_action)
{
    if (sigaction(sig, old_action, NULL) < 0) {
        printf("We can not restore action!\n");
        exit(1);
    }
    printf("We restored action for signal %d!\n", sig);
}

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0) self_int = 1;
        else sleep_sec = atoi(argv[i]);
    }

    struct sigaction new_usr1, old_usr1;
    printf("Sigact pid=%d (сон в обработчике %d с%s)\n", getpid(), sleep_sec,
           self_int ? ", вариант с kill(getpid(), SIGINT)" : "");

    /* фоновый процесс из неинтерактивного shell стартует с SIGINT = SIG_IGN,
       поэтому явно возвращаем действие по умолчанию */
    signal(SIGINT, SIG_DFL);

    set_action(SIGUSR1, &new_usr1, &old_usr1);          /* как в скелете */
    sighandler_fn old_usr2 = mysig(SIGUSR2, sig_handler); /* через mysig */
    if (old_usr2 == SIG_ERR) { printf("mysig failed\n"); return 1; }
    printf("Action for signal %d is setted (mysig, старый обработчик = %s)\n", SIGUSR2,
           old_usr2 == SIG_DFL ? "SIG_DFL" : "другой");

    /* ждём сигналы: каждый пришедший прерывает sleep */
    int left = sleep_sec;
    while ((left = sleep(left)) > 0)
        ;

    restore_action(SIGUSR1, &old_usr1);
    mysig(SIGUSR2, old_usr2);
    printf("We restored action for signal %d!\n", SIGUSR2);
    printf("sigact: нормальное завершение\n");
    return 0;
}
