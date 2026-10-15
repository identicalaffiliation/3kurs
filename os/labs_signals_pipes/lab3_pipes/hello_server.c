/*
 * hello_server.c — Уровень 1 (B): разбор скелета Server.c
 *
 * Сервер создаёт FIFO (специальный файл типа p), открывает его на чтение
 * (open блокируется, пока FIFO не откроет на запись какой-нибудь процесс),
 * читает сообщение и выводит его на экран. Удаляет FIFO клиент.
 *
 * Исправлено относительно скелета: <sys/start.h> -> <sys/stat.h>,
 * mknode -> mknod; путь "fifo/1" требует каталога fifo, взят /tmp/fifo.hello.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FIFO    "/tmp/fifo.hello"
#define MAXBUFF 80

int main(void)
{
    int readfd;
    ssize_t n;
    char buff[MAXBUFF];                      /* буфер для чтения данных из FIFO */

    /* Создадим FIFO с правами на чтение и запись для всех */
    if (mkfifo(FIFO, 0666) < 0) {
        perror("Сервер: невозможно создать FIFO");
        exit(1);
    }
    printf("Сервер: FIFO %s создан, жду клиента...\n", FIFO);
    fflush(stdout);

    /* Получим доступ к FIFO */
    if ((readfd = open(FIFO, O_RDONLY)) < 0) {
        perror("Сервер: невозможно открыть FIFO");
        exit(1);
    }
    printf("Сервер: получено сообщение: ");
    fflush(stdout);

    /* Прочитаем сообщение и выведем его на экран */
    while ((n = read(readfd, buff, MAXBUFF)) > 0)
        if (write(1, buff, n) != n) { printf("Ошибка вывода\n"); exit(1); }

    /* Закроем FIFO, удаление FIFO — дело клиента */
    close(readfd);
    exit(0);
}
