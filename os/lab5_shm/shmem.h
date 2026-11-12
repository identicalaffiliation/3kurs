/*
 * shmem.h — Уровень 1 (A): общий заголовок сервера и клиента
 *
 * Используются: сегмент разделяемой памяти System V и группа из двух
 * семафоров System V.
 *   семафор 0 — блокировка разделяемой памяти (0 — свободна, 1 — занята);
 *   семафор 1 — синхронизация процессов («клиент начал», «сервер прочитал»).
 *
 * struct sembuf = { номер семафора, операция, флаги }:
 *   операция  > 0 — прибавить к семафору;
 *   операция  < 0 — вычесть (ждать, пока значение не станет достаточным);
 *   операция == 0 — ждать, пока значение семафора станет 0.
 */
#ifndef SHMEM_H
#define SHMEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define MAXBUFF 80
#define PERM    0666
#define KEY_PATH "shmem.h"   /* в методичке ftok("server", 'A') */

/* Структура данных в разделяемой памяти */
typedef struct mem_msg {
    int  segment;
    char buff[MAXBUFF];
} Message;

/* Ожидание начала выполнения клиента (сервер: семафор 1 минус 1) */
static struct sembuf proc_wait[1]  = { {1, -1, 0} };

/* Уведомление сервера о том, что клиент начал работу (семафор 1 плюс 1).
   В методичке флаг = 2; такого флага нет (есть IPC_NOWAIT и SEM_UNDO),
   поэтому здесь 0. */
static struct sembuf proc_start[1] = { {1, 1, 0} };

/* Блокировка разделяемой памяти: дождаться, что семафор 0 равен 0,
   и сразу сделать его 1. Обе операции выполняются атомарно. */
static struct sembuf mem_lock[2]   = { {0, 0, 0}, {0, 1, 0} };

/* Освобождение ресурса: семафор 0 минус 1 */
static struct sembuf mem_unlock[1] = { {0, -1, 0} };

#endif
