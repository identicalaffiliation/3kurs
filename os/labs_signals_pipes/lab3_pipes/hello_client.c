#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FIFO "/tmp/fifo.hello"           

int main(void)
{
    int writefd;
    const char *msg = "Здравствуй, Мир!\n";
    ssize_t len = strlen(msg);

    if ((writefd = open(FIFO, O_WRONLY)) < 0) {
        perror("Клиент: невозможно открыть FIFO");
        exit(1);
    }
    if (write(writefd, msg, len) != len) {
        printf("Ошибка записи\n");
        exit(1);
    }
    printf("Клиент: отправлено %zd байт\n", len);
    close(writefd);                         

    if (unlink(FIFO) < 0) {                
        perror("Клиент: невозможно удалить FIFO");
        exit(1);
    }
    printf("Клиент: FIFO удалён\n");
    exit(0);
}
