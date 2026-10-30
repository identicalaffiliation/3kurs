#include <pthread.h>
#include <semaphore.h>
#include "sem_compat.h"   
#include <stdio.h>
#include <unistd.h>

#define BUF_SIZE 3
#define ITEMS    8

static int buf[BUF_SIZE];
static int in = 0, out = 0;
static sem_t empty, full;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static void *producer(void *a)
{
    (void)a;
    for (int i = 1; i <= ITEMS; i++) {
        sem_wait(&empty);                   
        pthread_mutex_lock(&mtx);
        buf[in] = i;
        in = (in + 1) % BUF_SIZE;
        int v; sem_getvalue(&full, &v);
        printf("Производитель: положил %d   (заполнено до post: %d)\n", i, v);
        pthread_mutex_unlock(&mtx);
        sem_post(&full);                    
    }
    return NULL;
}

static void *consumer(void *a)
{
    (void)a;
    for (int i = 0; i < ITEMS; i++) {
        sem_wait(&full);                   
        pthread_mutex_lock(&mtx);
        int x = buf[out];
        out = (out + 1) % BUF_SIZE;
        printf("    Потребитель: взял %d\n", x);
        pthread_mutex_unlock(&mtx);
        sem_post(&empty);                   
        usleep(50000);                     
    }
    return NULL;
}

int main(void)
{
    sem_init(&empty, 0, BUF_SIZE);
    sem_init(&full,  0, 0);

    int v;
    sem_getvalue(&empty, &v);
    printf("Начальные значения: empty = %d, ", v);
    sem_getvalue(&full, &v);
    printf("full = %d\n\n", v);

    pthread_t p, c;
    pthread_create(&p, NULL, producer, NULL);
    pthread_create(&c, NULL, consumer, NULL);
    pthread_join(p, NULL);
    pthread_join(c, NULL);

    sem_getvalue(&empty, &v);
    printf("\nКонечные значения: empty = %d, ", v);
    sem_getvalue(&full, &v);
    printf("full = %d\n", v);

    sem_destroy(&empty);
    sem_destroy(&full);
    return 0;
}
