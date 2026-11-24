
#define _GNU_SOURCE
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

static pthread_t t1, t2;

static long my_tid(void)
{
#ifdef __APPLE__
    uint64_t id;
    pthread_threadid_np(NULL, &id);
    return (long)id;
#else
    return (long)gettid();
#endif
}

static void usr1_handler(int sig)
{
    printf("  [нить 2, tid %ld] получен сигнал %d (SIGUSR1) — выходим через pthread_exit\n",
           my_tid(), sig);
    pthread_exit(NULL);                   
}

static void *thread2(void *a)
{
    (void)a;
    printf("  [нить 2] запущена, tid = %ld\n", my_tid());
    for (int i = 1; i <= 30; i++) {
        sleep(1);
        printf("  [нить 2] работаю... %d\n", i);
    }
    return NULL;
}

static void *thread1(void *a)
{
    int mode = *(int *)a;
    printf("  [нить 1] запущена, tid = %ld\n", my_tid());
    if (mode == 0) { sleep(30); return NULL; }

    sleep(2);
    printf("  [нить 1] посылаю SIGUSR1 во вторую нить: pthread_kill(t2, SIGUSR1)\n");
    int rc = pthread_kill(t2, SIGUSR1);
    printf("  [нить 1] pthread_kill вернул %d\n", rc);
    return NULL;
}

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    int mode = argc > 1 ? atoi(argv[1]) : 2;
    printf("Процесс pid = %d, режим %d\n", getpid(), mode);

    if (mode == 2) {
        struct sigaction sa = {0};
        sa.sa_handler = usr1_handler;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGUSR1, &sa, NULL);
    }

    pthread_create(&t2, NULL, thread2, NULL);
    pthread_create(&t1, NULL, thread1, &mode);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("main: обе нити завершены, процесс продолжает работу и выходит нормально\n");
    return 0;
}
