/*
 * sig_father.c — Уровень 2 (A), ненадёжные сигналы (функция signal)
 *
 *   SIGUSR1, SIGUSR2 — собственные обработчики;
 *   SIGINT           — действие по умолчанию;
 *   SIGCHLD          — игнорируется (сын не становится зомби).
 * Порождает сына sig_son (через execl) и ждёт сигналов.
 * В обработчиках диспозиция восстанавливается повторным вызовом signal(),
 * как того требует модель ненадёжных сигналов.
 *
 * Сигналы можно слать и с терминала:
 *   ./sig_father &
 *   kill -s USR2 <PID>;  kill -s USR1 <PID>;  kill -s INT <PID>
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static volatile sig_atomic_t got = 0;   /* сколько сигналов получено */

static void sig_handler_usr1(int sig);
static void sig_handler_usr2(int sig);

/* восстановление диспозиции: для ненадёжных сигналов после срабатывания
   обработчика ядро могло сбросить её в SIG_DFL, поэтому ставим заново */
static void restore_signals(void)
{
    signal(SIGUSR1, sig_handler_usr1);
    signal(SIGUSR2, sig_handler_usr2);
    signal(SIGINT,  SIG_DFL);
    signal(SIGCHLD, SIG_IGN);
}

static void sig_handler_usr1(int sig)
{
    printf("usr1 handler: получен сигнал %d — %s\n", sig,
           sig == SIGUSR1 ? "Our signal! (SIGUSR1)" : "Not our signal");
    printf("              pid=%d ppid=%d\n", getpid(), getppid());
    got++;
    restore_signals();
}

static void sig_handler_usr2(int sig)
{
    printf("usr2 handler: получен сигнал %d — %s\n", sig,
           sig == SIGUSR2 ? "Our signal! (SIGUSR2)" : "Not our signal");
    printf("              pid=%d ppid=%d\n", getpid(), getppid());
    got++;
    restore_signals();
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("sig_father is starting! pid=%d\n", getpid());

    restore_signals();                       /* установка диспозиций */

    pid_t son_pid = fork();
    if (son_pid < 0) { perror("fork"); exit(1); }
    if (son_pid == 0) {
        execl("./sig_son", "sig_son", (char *)NULL);
        perror("execl sig_son");             /* сюда попадаем только при ошибке */
        _exit(1);
    }

    /* ждём сигналы: pause() возвращается после каждого обработанного сигнала.
       SIGINT обрабатывается по умолчанию и завершает процесс. */
    for (;;) {
        int before = got;
        pause();
        printf("sig_father: сигнал %s (всего обработано: %d), ждём дальше...\n",
               got > before ? "получен" : "НЕ получен", (int)got);
    }
    return 0;
}
