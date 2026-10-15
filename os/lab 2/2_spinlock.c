#include <pthread.h>
#include "spin_compat.h"  
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define NTHREADS 2
#define ITERS    5000000

static long counter;
static pthread_spinlock_t spin;
static pthread_mutex_t    mtx = PTHREAD_MUTEX_INITIALIZER;

static void *spin_worker(void *a)
{
    (void)a;
    for (int i = 0; i < ITERS; i++) {
        pthread_spin_lock(&spin);
        counter++;
        pthread_spin_unlock(&spin);
    }
    return NULL;
}

static void *mutex_worker(void *a)
{
    (void)a;
    for (int i = 0; i < ITERS; i++) {
        pthread_mutex_lock(&mtx);
        counter++;
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void bench(const char *name, void *(*fn)(void *))
{
    pthread_t th[NTHREADS];
    counter = 0;
    double t0 = now();
    for (int i = 0; i < NTHREADS; i++) pthread_create(&th[i], NULL, fn, NULL);
    for (int i = 0; i < NTHREADS; i++) pthread_join(th[i], NULL);
    double dt = now() - t0;
    printf("%-9s: counter = %ld (ожидалось %ld), время = %.3f c\n",
           name, counter, (long)NTHREADS * ITERS, dt);
}

static void *try_worker(void *a)
{
    (void)a;
    int rc = pthread_spin_trylock(&spin);
    printf("pthread_spin_trylock из другого потока: rc = %d (%s)\n",
           rc, rc == EBUSY ? "EBUSY — спинлок занят" : strerror(rc));
    if (rc == 0) pthread_spin_unlock(&spin);
    return NULL;
}

int main(void)
{
    /* PTHREAD_PROCESS_PRIVATE — только для потоков одного процесса */
    int rc = pthread_spin_init(&spin, PTHREAD_PROCESS_PRIVATE);
    if (rc) { fprintf(stderr, "pthread_spin_init: %s\n", strerror(rc)); return 1; }

    printf("=== 1. Короткая критическая секция: спинлок vs мьютекс ===\n");
    bench("Спинлок", spin_worker);
    bench("Мьютекс", mutex_worker);

    printf("\n=== 2. pthread_spin_trylock ===\n");
    pthread_t t;
    pthread_spin_lock(&spin);
    pthread_create(&t, NULL, try_worker, NULL);
    pthread_join(t, NULL);
    pthread_spin_unlock(&spin);
    pthread_create(&t, NULL, try_worker, NULL);
    pthread_join(t, NULL);

    pthread_spin_destroy(&spin);
    pthread_mutex_destroy(&mtx);
    return 0;
}
