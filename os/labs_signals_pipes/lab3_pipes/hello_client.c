/*
 * hello_client.c — Уровень 1 (B): разбор скелета Client.c
 *
 * Клиент открывает существующий FIFO на запись, пишет сообщение,
 * закрывает FIFO (сервер получит конец файла) и удаляет его unlink'ом.
 *
 * В скелете длина сообщения задана числом 18, но в UTF-8 русская буква
 * занимает 2 байта, поэтому длина считается через strlen.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FIFO "/tmp/fifo.hello"               /* соглашение об имени FIFO */

int main(void)
{
    int writefd;
    const char *msg = "Здравствуй, Мир!\n";
    ssize_t len = strlen(msg);

    /* Получим доступ к FIFO */
    if ((writefd = open(FIFO, O_WRONLY)) < 0) {
        perror("Клиент: невозможно открыть FIFO");
        exit(1);
    }
    /* Передадим сообщение серверу */
    if (write(writefd, msg, len) != len) {
        printf("Ошибка записи\n");
        exit(1);
    }
    printf("Клиент: отправлено %zd байт\n", len);
    close(writefd);                          /* закроем FIFO */

    if (unlink(FIFO) < 0) {                  /* удалим FIFO */
        perror("Клиент: невозможно удалить FIFO");
        exit(1);
    }
    printf("Клиент: FIFO удалён\n");
    exit(0);
}
