/*
 * l2_common.h — Уровень 2 (A): сервер и несколько клиентов
 *
 * Запрос клиента — тип 1, ответ сервера — тип 2 (как в задании).
 *
 * Проблема: если клиентов несколько и все ждут ответ типа 2, ответ,
 * предназначенный одному клиенту, может забрать другой. Поэтому по
 * умолчанию ответ адресуется конкретному клиенту: тип ответа = PID
 * клиента (стандартный приём для очередей System V; PID > 2, так что
 * с типом запроса 1 не пересекается).
 * Чтобы вернуть ровно «тип 2» из методички, соберите с -DREPLY_TYPE_2.
 */
#ifndef L2_COMMON_H
#define L2_COMMON_H

#include "mesg.h"

#define REQ_TYPE 1L

#ifdef REPLY_TYPE_2
#  define REPLY_TYPE(pid) 2L
#else
#  define REPLY_TYPE(pid) ((long)(pid))
#endif

/* Данные запроса/ответа: кто отправил и текст */
typedef struct {
    long mtype;
    pid_t pid;               /* PID клиента — куда отвечать */
    char text[MAXBUFF];
} Request;

#define REQ_SIZE (sizeof(Request) - sizeof(long))   /* размер без mtype */

#endif
