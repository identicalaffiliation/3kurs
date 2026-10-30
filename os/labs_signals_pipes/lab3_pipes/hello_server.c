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
    char buff[MAXBUFF];                      

    if (mkfifo(FIFO, 0666) < 0) {
        perror("Сервер: невозможно создать FIFO");
        exit(1);
    }
    printf("Сервер: FIFO %s создан, жду клиента...\n", FIFO);
    fflush(stdout);

    if ((readfd = open(FIFO, O_RDONLY)) < 0) {
        perror("Сервер: невозможно открыть FIFO");
        exit(1);
    }
    printf("Сервер: получено сообщение: ");
    fflush(stdout);

    while ((n = read(readfd, buff, MAXBUFF)) > 0)
        if (write(1, buff, n) != n) { printf("Ошибка вывода\n"); exit(1); }

    close(readfd);
    exit(0);
}
