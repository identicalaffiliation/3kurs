/* mesg.h — структурированные сообщения для задания 3 (B)
 *
 * Канал (FIFO) — это поток байтов без границ сообщений. Чтобы передавать
 * структурированные данные, каждое сообщение снабжается заголовком:
 *   mesg_len  — длина поля данных,
 *   mesg_type — тип сообщения.
 * Получатель сначала читает заголовок фиксированного размера, а затем
 * ровно mesg_len байт данных — так границы сообщений восстанавливаются.
 */
#ifndef MESG_H
#define MESG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FIFO1 "/tmp/fifo.1"
#define FIFO2 "/tmp/fifo.2"
#define MAXMESGDATA 4096

struct mymesg {
    long mesg_len;                  /* длина данных */
    long mesg_type;                 /* тип сообщения */
    char mesg_data[MAXMESGDATA];
};
#define MESGHDRSIZE (2 * sizeof(long))

/* типы сообщений */
#define MSG_REQUEST 1               /* клиент -> сервер: имя файла */
#define MSG_DATA    2               /* сервер -> клиент: порция содержимого файла */
#define MSG_ERROR   3               /* сервер -> клиент: текст ошибки */
#define MSG_END     4               /* сервер -> клиент: конец ответа (len = 0) */

static inline const char *type_name(long t)
{
    switch (t) {
    case MSG_REQUEST: return "REQUEST";
    case MSG_DATA:    return "DATA";
    case MSG_ERROR:   return "ERROR";
    case MSG_END:     return "END";
    default:          return "?";
    }
}

static inline ssize_t mesg_send(int fd, struct mymesg *mptr)
{
    return write(fd, mptr, MESGHDRSIZE + mptr->mesg_len);
}

/* возвращает длину данных, 0 — сообщение без данных, -1 — конец файла/ошибка */
static inline ssize_t mesg_recv(int fd, struct mymesg *mptr)
{
    ssize_t n;
    size_t len;

    if ((n = read(fd, mptr, MESGHDRSIZE)) == 0)
        return -1;                                  /* конец файла */
    if (n != (ssize_t)MESGHDRSIZE) {
        fprintf(stderr, "mesg_recv: ошибка чтения заголовка (%zd байт)\n", n);
        return -1;
    }
    if ((len = mptr->mesg_len) > 0) {
        if (len > MAXMESGDATA) { fprintf(stderr, "mesg_recv: слишком длинное сообщение\n"); exit(1); }
        if ((n = read(fd, mptr->mesg_data, len)) != (ssize_t)len) {
            fprintf(stderr, "mesg_recv: ожидалось %zu байт данных, получено %zd\n", len, n);
            exit(1);
        }
    }
    return len;
}

#endif
