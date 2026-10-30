#include "common.h"

int main(void)
{
    int readfd, writefd;

    if (mkfifo(FIFO1, 0666) < 0) {
        perror("Server: can not create FIFO1");
        exit(1);
    }
    printf("Server: FIFO1 is created!\n");
    if (mkfifo(FIFO2, 0666) < 0) {
        unlink(FIFO1);
        perror("Server: can not create FIFO2");
        exit(1);
    }
    printf("Server: FIFO2 is created!\n");
    fflush(stdout);

    if ((readfd = open(FIFO1, O_RDONLY)) < 0) {
        perror("Server: can not open FIFO1 for read"); exit(1);
    }
    printf("Server: FIFO1 is opened for read and readfd=%d\n", readfd);
    if ((writefd = open(FIFO2, O_WRONLY)) < 0) {
        perror("Server: can not open FIFO2 for write"); exit(1);
    }
    printf("Server: FIFO2 is opened for write and writefd=%d\n", writefd);
    fflush(stdout);

    server(readfd, writefd);

    close(readfd);
    close(writefd);
    printf("Server is terminated!\n");
    exit(0);
}
