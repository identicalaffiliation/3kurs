/*
 * l3_shm.h — Уровень 3 (A): файловый клиент-сервер через разделяемую память
 *
 * Сегмент разделяемой памяти System V — «почтовый ящик» на одно сообщение.
 * Семафоры System V (группа из 4) согласуют, кто и когда к нему обращается:
 *
 *   SEM_FREE (нач. 1)  — сервер свободен; клиенты занимают его по очереди,
 *                        поэтому одновременно может прийти много клиентов;
 *   SEM_REQ  (нач. 0)  — клиент положил имя файла;
 *   SEM_RESP (нач. 0)  — сервер положил порцию ответа;
 *   SEM_ACK  (нач. 0)  — клиент забрал порцию, ящик снова пуст.
 *
 * Клиент:  P(FREE) -> имя в память -> V(REQ) ->
 *          { P(RESP) -> вывести -> V(ACK) } до END/ERROR -> V(FREE)
 * Сервер:  { P(REQ) -> открыть файл -> { порция -> V(RESP) -> P(ACK) } -> END } ...
 */
#ifndef L3_SHM_H
#define L3_SHM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define KEY_PATH "l3_shm.h"
#define KEY_ID   'S'
#define PERM     0666
#define CHUNK    1024

enum { SEM_FREE, SEM_REQ, SEM_RESP, SEM_ACK, NSEMS };
enum { T_REQUEST = 1, T_DATA, T_ERROR, T_END };

typedef struct {
    pid_t client;           /* PID клиента, который сейчас обслуживается */
    int  type;              /* T_REQUEST / T_DATA / T_ERROR / T_END */
    int  len;               /* длина данных в data */
    char data[CHUNK];
} ShmMsg;

/* В Linux union semun нужно объявить самому, в macOS он уже есть в <sys/sem.h> */
#ifdef __linux__
union semun { int val; struct semid_ds *buf; unsigned short *array; };
#endif

/* P и V для одного семафора группы; при EINTR операция повторяется */
static inline int sem_op(int semid, int num, int op, int flg)
{
    struct sembuf sb = { (unsigned short)num, (short)op, (short)flg };
    while (semop(semid, &sb, 1) < 0) {
        if (errno != EINTR) return -1;
    }
    return 0;
}
#define P(id, n)      sem_op(id, n, -1, 0)
#define V(id, n)      sem_op(id, n, +1, 0)

#endif
