#include "mesg.h"

static void server(int readfd, int writefd)
{
    struct mymesg mesg;
    ssize_t n;

    if ((n = mesg_recv(readfd, &mesg)) <= 0 || mesg.mesg_type != MSG_REQUEST) {
        printf("Server: запрос не получен\n");
        return;
    }
    mesg.mesg_data[n] = '\0';
    printf("Server: получено сообщение type=%s len=%zd: \"%s\"\n",
           type_name(mesg.mesg_type), n, mesg.mesg_data);

    int fd = open(mesg.mesg_data, O_RDONLY);
    if (fd < 0) {
        char path[MAXMESGDATA];
        strcpy(path, mesg.mesg_data);
        mesg.mesg_type = MSG_ERROR;
        mesg.mesg_len = snprintf(mesg.mesg_data, MAXMESGDATA,
                                 "невозможно открыть %s: %s", path, strerror(errno));
        mesg_send(writefd, &mesg);
        printf("Server: отправлено ERROR\n");
    } else {
        int parts = 0;
        mesg.mesg_type = MSG_DATA;
        while ((n = read(fd, mesg.mesg_data, MAXMESGDATA)) > 0) {
            mesg.mesg_len = n;
            mesg_send(writefd, &mesg);
            parts++;
        }
        close(fd);
        printf("Server: отправлено сообщений DATA: %d\n", parts);
    }
    mesg.mesg_type = MSG_END;              
    mesg.mesg_len = 0;
    mesg_send(writefd, &mesg);
    printf("Server: отправлено END\n");
}

int main(void)
{
    int readfd, writefd;
    setvbuf(stdout, NULL, _IONBF, 0);

    if (mkfifo(FIFO1, 0666) < 0) { perror("Server: can not create FIFO1"); exit(1); }
    printf("Server: FIFO1 is created!\n");
    if (mkfifo(FIFO2, 0666) < 0) {
        unlink(FIFO1);
        perror("Server: can not create FIFO2"); exit(1);
    }
    printf("Server: FIFO2 is created!\n");

    if ((readfd = open(FIFO1, O_RDONLY)) < 0) { perror("Server: open FIFO1"); exit(1); }
    printf("Server: FIFO1 is opened for read and readfd=%d\n", readfd);
    if ((writefd = open(FIFO2, O_WRONLY)) < 0) { perror("Server: open FIFO2"); exit(1); }
    printf("Server: FIFO2 is opened for write and writefd=%d\n", writefd);

    server(readfd, writefd);

    close(readfd);
    close(writefd);
    printf("Server is terminated!\n");
    exit(0);
}
