/*
 * l3_msg.h — Уровень 3 (A): общая часть client_msg.c и server_msg.c
 *
 * Здесь дописаны функции mesg_send и mesg_recv из скелета.
 *
 * Хитрость: в struct mymesg первым полем идёт mesg_len, а msgsnd/msgrcv
 * требуют, чтобы буфер начинался с long-типа. Поэтому в ядро передаётся
 * указатель не на начало структуры, а на поле mesg_type: с него
 * начинается «правильный» буфер {long mtype; char data[]}.
 * Поле mesg_len в очередь не передаётся: длину данных msgrcv возвращает сам.
 */
#ifndef L3_MSG_H
#define L3_MSG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MQ_KEY1 1234L        /* клиент -> сервер */
#define MQ_KEY2 2345L        /* сервер -> клиент */

/* В методичке 4096. В macOS одно сообщение не может быть длиннее
   2048 байт (kern.sysv.msgmax), поэтому там берём 1024. */
#ifdef __APPLE__
#  define MAXMESGDATA 1024
#else
#  define MAXMESGDATA 4096
#endif

struct mymesg {
    long mesg_len;                   /* длина данных (в очередь не идёт) */
    long mesg_type;                  /* тип сообщения (> 0) */
    char mesg_data[MAXMESGDATA];
};

/* Отправить сообщение: тип + mesg_len байт данных.
   mesg_len = 0 допустимо — пустое сообщение служит признаком конца. */
static inline ssize_t mesg_send(int id, struct mymesg *mptr)
{
    return msgsnd(id, &(mptr->mesg_type), mptr->mesg_len, 0);
}

/* Принять сообщение типа mptr->mesg_type.
   Возвращает длину данных (0 — пустое сообщение-конец), -1 — ошибка. */
static inline ssize_t mesg_recv(int id, struct mymesg *mptr)
{
    ssize_t n = msgrcv(id, &(mptr->mesg_type), MAXMESGDATA, mptr->mesg_type, 0);
    mptr->mesg_len = n;
    return n;
}

#endif
