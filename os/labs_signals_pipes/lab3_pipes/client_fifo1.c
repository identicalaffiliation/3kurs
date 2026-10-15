/*
 * client_fifo1.c — Уровень 2 (B): клиент на именованных каналах
 * Запуск: echo /etc/hostname | ./client_fifo1
 */
#include "common.h"

int main(void)
{
    int readfd, writefd;

    if ((writefd = open(FIFO1, O_WRONLY)) < 0) {
        perror("Client: can not open FIFO1 for write"); exit(1);
    }
    fprintf(stderr, "Client: FIFO1 is opened for write writefd=%d\n", writefd);
    if ((readfd = open(FIFO2, O_RDONLY)) < 0) {
        perror("Client: can not open FIFO2 for read"); exit(1);
    }
    fprintf(stderr, "Client: FIFO2 is opened for read readfd=%d\n", readfd);

    client(readfd, writefd);

    close(readfd);
    close(writefd);

    if (unlink(FIFO1) < 0) { perror("Client: can not delete FIFO1"); exit(1); }
    fprintf(stderr, "Client: FIFO1 is deleted!\n");
    if (unlink(FIFO2) < 0) { perror("Client: can not delete FIFO2"); exit(1); }
    fprintf(stderr, "Client: FIFO2 is deleted!\n");
    fprintf(stderr, "Client is terminated!\n");
    exit(0);
}
