#define _GNU_SOURCE

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define THREADS 4
#define ITERATIONS 1000000

static long long counter;

/* ---------- Mutex ---------- */

static pthread_mutex_t mutex;

static void *mutex_worker(void *arg)
{
    (void)arg;

    for (int i = 0; i < ITERATIONS; ++i) {
        pthread_mutex_lock(&mutex);
        ++counter;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

/* ---------- Spinlock ---------- */

static pthread_spinlock_t spinlock;

static void *spin_worker(void *arg)
{
    (void)arg;

    for (int i = 0; i < ITERATIONS; ++i) {
        pthread_spin_lock(&spinlock);
        ++counter;
        pthread_spin_unlock(&spinlock);
    }

    return NULL;
}

/* ---------- POSIX semaphore ---------- */

static sem_t semaphore;

static void *semaphore_worker(void *arg)
{
    (void)arg;

    for (int i = 0; i < ITERATIONS; ++i) {
        sem_wait(&semaphore);
        ++counter;
        sem_post(&semaphore);
    }

    return NULL;
}

static double now_seconds(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

typedef void *(*worker_fn)(void *);

static long long run_test(worker_fn worker, const char *name)
{
    pthread_t threads[THREADS];

    counter = 0;

    double start = now_seconds();

    for (int i = 0; i < THREADS; ++i) {
        int rc = pthread_create(&threads[i], NULL, worker, NULL);
        if (rc != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(rc));
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < THREADS; ++i) {
        int rc = pthread_join(threads[i], NULL);
        if (rc != 0) {
            fprintf(stderr, "pthread_join: %s\n", strerror(rc));
            exit(EXIT_FAILURE);
        }
    }

    double elapsed = now_seconds() - start;

    printf("%-12s counter = %lld, expected = %lld, time = %.6f s\n",
           name,
           counter,
           (long long)THREADS * ITERATIONS,
           elapsed);

    return counter;
}

int main(void)
{
    const long long expected = (long long)THREADS * ITERATIONS;

    printf("Linux synchronization lab\n");
    printf("Threads: %d, iterations per thread: %d\n\n",
           THREADS, ITERATIONS);

    /* Mutex */
    int rc = pthread_mutex_init(&mutex, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_mutex_init: %s\n", strerror(rc));
        return EXIT_FAILURE;
    }

    long long mutex_result = run_test(mutex_worker, "mutex");

    rc = pthread_mutex_destroy(&mutex);
    if (rc != 0) {
        fprintf(stderr, "pthread_mutex_destroy: %s\n", strerror(rc));
        return EXIT_FAILURE;
    }

    /* Spinlock */
    rc = pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);
    if (rc != 0) {
        fprintf(stderr, "pthread_spin_init: %s\n", strerror(rc));
        return EXIT_FAILURE;
    }

    long long spin_result = run_test(spin_worker, "spinlock");

    rc = pthread_spin_destroy(&spinlock);
    if (rc != 0) {
        fprintf(stderr, "pthread_spin_destroy: %s\n", strerror(rc));
        return EXIT_FAILURE;
    }

    /* Unnamed POSIX semaphore */
    if (sem_init(&semaphore, 0, 1) != 0) {
        perror("sem_init");
        return EXIT_FAILURE;
    }

    long long semaphore_result = run_test(semaphore_worker, "semaphore");

    if (sem_destroy(&semaphore) != 0) {
        perror("sem_destroy");
        return EXIT_FAILURE;
    }

    printf("\nCheck:\n");
    printf("mutex:     %s\n", mutex_result == expected ? "OK" : "ERROR");
    printf("spinlock:  %s\n", spin_result == expected ? "OK" : "ERROR");
    printf("semaphore: %s\n", semaphore_result == expected ? "OK" : "ERROR");

    return EXIT_SUCCESS;
}
