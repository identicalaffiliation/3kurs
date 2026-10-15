/*
 * spin_compat.h — спин-блокировки POSIX для macOS
 *
 * В macOS нет pthread_spinlock_t и функций pthread_spin_* (это
 * необязательная часть POSIX, Apple её не реализует). На Linux этот
 * файл ничего не делает — используются настоящие функции из <pthread.h>.
 * На macOS спинлок реализован так же, как в ядре/glibc: атомарный флаг
 * и бесконечный цикл «проверить-и-установить» (test-and-set).
 */
#ifndef SPIN_COMPAT_H
#define SPIN_COMPAT_H

#include <pthread.h>

#if defined(__APPLE__) || defined(LAB_FORCE_COMPAT)
#include <stdatomic.h>
#include <errno.h>

typedef struct { atomic_flag flag; } lab_spinlock_t;
#define pthread_spinlock_t   lab_spinlock_t
#define pthread_spin_init    lab_spin_init
#define pthread_spin_destroy lab_spin_destroy
#define pthread_spin_lock    lab_spin_lock
#define pthread_spin_trylock lab_spin_trylock
#define pthread_spin_unlock  lab_spin_unlock

static inline int lab_spin_init(lab_spinlock_t *l, int pshared)
{
    (void)pshared;
    atomic_flag_clear(&l->flag);
    return 0;
}
static inline int lab_spin_destroy(lab_spinlock_t *l) { (void)l; return 0; }

static inline int lab_spin_lock(lab_spinlock_t *l)
{
    while (atomic_flag_test_and_set_explicit(&l->flag, memory_order_acquire))
        ;                                   /* крутимся, пока ресурс занят */
    return 0;
}
static inline int lab_spin_trylock(lab_spinlock_t *l)
{
    return atomic_flag_test_and_set_explicit(&l->flag, memory_order_acquire) ? EBUSY : 0;
}
static inline int lab_spin_unlock(lab_spinlock_t *l)
{
    atomic_flag_clear_explicit(&l->flag, memory_order_release);
    return 0;
}
#endif

#endif
