/* common.h — общие функции клиента и сервера (задания 2,3 A и 2 B) */
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAXLINE 4096
#define FIFO1 "/tmp/fifo.1"     /* клиент -> сервер */
#define FIFO2 "/tmp/fifo.2"     /* сервер -> клиент */

/* Клиент: читает полное имя файла из stdin, пишет его в канал,
   затем выводит в stdout всё, что пришло от сервера. */
static inline void client(int readfd, int writefd)
{
    char buff[MAXLINE];
    ssize_t n;

    fprintf(stderr, "Клиент: введите полное имя файла: ");
    if (fgets(buff, MAXLINE, stdin) == NULL) return;
    size_t len = strlen(buff);
    if (len > 0 && buff[len - 1] == '\n') len--;      /* убираем перевод строки */
    fprintf(stderr, "%.*s\n", (int)len, buff);       /* эхо (удобно при вводе из файла) */

    if (write(writefd, buff, len) != (ssize_t)len) { perror("client write"); return; }

    while ((n = read(readfd, buff, MAXLINE)) > 0)    /* ответ сервера -> stdout */
        if (write(STDOUT_FILENO, buff, n) != n) { perror("client stdout"); return; }
}

/* Сервер: читает имя файла из канала, открывает файл и передаёт его
   содержимое в канал; при ошибке передаёт сообщение об ошибке. */
static inline void server(int readfd, int writefd)
{
    char buff[MAXLINE + 1];
    ssize_t n;
    int fd;

    if ((n = read(readfd, buff, MAXLINE)) <= 0) {
        fprintf(stderr, "Сервер: имя файла не получено\n");
        return;
    }
    buff[n] = '\0';
    fprintf(stderr, "Сервер: запрошен файл \"%s\"\n", buff);

    if ((fd = open(buff, O_RDONLY)) < 0) {
        char err[MAXLINE + 128];
        int m = snprintf(err, sizeof err, "Ошибка: невозможно открыть %s: %s\n",
                         buff, strerror(errno));
        if (write(writefd, err, m) != m) perror("server write");
    } else {
        while ((n = read(fd, buff, MAXLINE)) > 0)
            if (write(writefd, buff, n) != n) { perror("server write"); break; }
        close(fd);
    }
}

#endif
