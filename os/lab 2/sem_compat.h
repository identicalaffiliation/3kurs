
#ifndef SEM_COMPAT_H
#define SEM_COMPAT_H

#include <pthread.h>
#include <semaphore.h>

#if defined(__APPLE__) || defined(LAB_FORCE_COMPAT)
typedef struct {
    pthread_mutex_t m;
    pthread_cond_t  c;
    unsigned int    value;          /* значение семафора, никогда не < 0 */
} lab_sem_t;

#define sem_t        lab_sem_t
#define sem_init     lab_sem_init
#define sem_destroy  lab_sem_destroy
#define sem_wait     lab_sem_wait
#define sem_post     lab_sem_post
#define sem_getvalue lab_sem_getvalue

static inline int lab_sem_init(lab_sem_t *s, int pshared, unsigned int value)
{
    (void)pshared;                  /* поддерживается только семафор для нитей */
    s->value = value;
    pthread_mutex_init(&s->m, NULL);
    pthread_cond_init(&s->c, NULL);
    return 0;
}
static inline int lab_sem_destroy(lab_sem_t *s)
{
    pthread_mutex_destroy(&s->m);
    pthread_cond_destroy(&s->c);
    return 0;
}
static inline int lab_sem_wait(lab_sem_t *s)       /* операция P */
{
    pthread_mutex_lock(&s->m);
    while (s->value == 0)                          /* значение 0 -> блокируемся */
        pthread_cond_wait(&s->c, &s->m);
    s->value--;
    pthread_mutex_unlock(&s->m);
    return 0;
}
static inline int lab_sem_post(lab_sem_t *s)       /* операция V */
{
    pthread_mutex_lock(&s->m);
    s->value++;
    pthread_cond_signal(&s->c);                    /* будим одного ожидающего */
    pthread_mutex_unlock(&s->m);
    return 0;
}
static inline int lab_sem_getvalue(lab_sem_t *s, int *valp)
{
    pthread_mutex_lock(&s->m);
    *valp = (int)s->value;
    pthread_mutex_unlock(&s->m);
    return 0;
}
#endif

#endif
