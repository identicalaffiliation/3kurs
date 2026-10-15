/*
 * client_fifomsg.c — Уровень 3 (B): клиент со структурированными сообщениями
 *
 * Читает имя файла из stdin, отправляет его серверу сообщением REQUEST,
 * затем принимает сообщения DATA / ERROR до сообщения END.
 * Запуск: echo /etc/hostname | ./client_fifomsg
 */
#include "mesg.h"

static void client(int readfd, int writefd)
{
    struct mymesg mesg;
    ssize_t n;

    fprintf(stderr, "Client: введите полное имя файла: ");
    if (fgets(mesg.mesg_data, MAXMESGDATA, stdin) == NULL) return;
    size_t len = strlen(mesg.mesg_data);
    if (len > 0 && mesg.mesg_data[len - 1] == '\n') len--;
    fprintf(stderr, "%.*s\n", (int)len, mesg.mesg_data);

    mesg.mesg_len  = len;
    mesg.mesg_type = MSG_REQUEST;
    mesg_send(writefd, &mesg);

    int data_msgs = 0;
    while ((n = mesg_recv(readfd, &mesg)) >= 0) {
        if (mesg.mesg_type == MSG_DATA) {
            data_msgs++;
            if (write(STDOUT_FILENO, mesg.mesg_data, n) != n) { perror("write"); return; }
        } else if (mesg.mesg_type == MSG_ERROR) {
            fprintf(stderr, "Client: сервер сообщил об ошибке: %.*s\n", (int)n, mesg.mesg_data);
        } else if (mesg.mesg_type == MSG_END) {
            fprintf(stderr, "Client: получено END, сообщений DATA: %d\n", data_msgs);
            break;
        }
    }
}

int main(void)
{
    int readfd, writefd;

    if ((writefd = open(FIFO1, O_WRONLY)) < 0) { perror("Client: can not open FIFO1 for write"); exit(1); }
    fprintf(stderr, "Client: FIFO1 is opened for write writefd=%d\n", writefd);
    if ((readfd = open(FIFO2, O_RDONLY)) < 0) { perror("Client: can not open FIFO2 for read"); exit(1); }
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
