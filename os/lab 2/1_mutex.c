/*
 * 1_mutex.c — демонстрация мьютекса pthread_mutex_t
 *
 * Несколько потоков увеличивают общий счётчик.
 * Режим 0: без синхронизации  -> гонка данных, итог меньше ожидаемого.
 * Режим 1: с мьютексом        -> итог всегда верный.
 * Дополнительно показаны pthread_mutex_trylock (EBUSY)
 * и коды ошибок мьютекса с проверкой ошибок (EDEADLK, EPERM).
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define NTHREADS 4
#define ITERS    1000000

static long counter = 0;
static pthread_mutex_t mtx;
static int use_mutex;

static void *worker(void *arg)
{
    (void)arg;
    for (int i = 0; i < ITERS; i++) {
        if (use_mutex) pthread_mutex_lock(&mtx);
        counter++;                       /* критическая секция */
        if (use_mutex) pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

static void run(int mode)
{
    pthread_t th[NTHREADS];
    counter = 0;
    use_mutex = mode;
    for (int i = 0; i < NTHREADS; i++)
        pthread_create(&th[i], NULL, worker, NULL);
    for (int i = 0; i < NTHREADS; i++)
        pthread_join(th[i], NULL);
    printf("%-18s: counter = %ld (ожидалось %ld)%s\n",
           mode ? "С мьютексом" : "Без синхронизации",
           counter, (long)NTHREADS * ITERS,
           counter == (long)NTHREADS * ITERS ? "  OK" : "  <-- гонка данных!");
}

/* поток пытается занять уже занятый мьютекс через trylock */
static void *try_worker(void *arg)
{
    int rc = pthread_mutex_trylock((pthread_mutex_t *)arg);
    printf("trylock из другого потока: rc = %d (%s)\n",
           rc, rc == EBUSY ? "EBUSY — мьютекс занят" : strerror(rc));
    if (rc == 0) pthread_mutex_unlock((pthread_mutex_t *)arg);
    return NULL;
}

/* поток пытается освободить чужой мьютекс */
static void *foreign_unlock(void *arg)
{
    int rc = pthread_mutex_unlock((pthread_mutex_t *)arg);
    printf("unlock чужого мьютекса: rc = %d (%s)\n",
           rc, rc == EPERM ? "EPERM — поток не владелец" : strerror(rc));
    return NULL;
}

int main(void)
{
    int rc = pthread_mutex_init(&mtx, NULL);   /* атрибуты по умолчанию */
    if (rc) { fprintf(stderr, "pthread_mutex_init: %s\n", strerror(rc)); return 1; }

    printf("=== 1. Гонка данных и её устранение мьютексом ===\n");
    run(0);
    run(1);

    printf("\n=== 2. pthread_mutex_trylock ===\n");
    pthread_t t;
    pthread_mutex_lock(&mtx);                       /* main владеет мьютексом */
    pthread_create(&t, NULL, try_worker, &mtx);
    pthread_join(t, NULL);
    pthread_mutex_unlock(&mtx);
    pthread_create(&t, NULL, try_worker, &mtx);     /* теперь свободен */
    pthread_join(t, NULL);

    printf("\n=== 3. Мьютекс с проверкой ошибок (PTHREAD_MUTEX_ERRORCHECK) ===\n");
    pthread_mutexattr_t attr;
    pthread_mutex_t emtx;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&emtx, &attr);

    pthread_mutex_lock(&emtx);
    rc = pthread_mutex_lock(&emtx);                 /* повторный захват */
    printf("повторный lock тем же потоком: rc = %d (%s)\n",
           rc, rc == EDEADLK ? "EDEADLK — уже занят этим потоком" : strerror(rc));
    pthread_create(&t, NULL, foreign_unlock, &emtx);
    pthread_join(t, NULL);
    pthread_mutex_unlock(&emtx);

    pthread_mutex_destroy(&emtx);
    pthread_mutexattr_destroy(&attr);
    pthread_mutex_destroy(&mtx);
    return 0;
}
