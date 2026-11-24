#include "mesg.h"

int main(void)
{
    Message message;
    key_t key;
    int msgid;
    ssize_t n;

    if ((key = ftok(KEY_PATH, 'A')) < 0) { perror("Невозможно получить ключ"); exit(1); }

    message.mtype = 1L;

    if ((msgid = msgget(key, PERM | IPC_CREAT)) < 0) { perror("Невозможно создать очередь"); exit(1); }
    printf("Сервер: ключ = 0x%x, msgid = %d, жду сообщение типа 1...\n", (unsigned)key, msgid);
    fflush(stdout);

    n = msgrcv(msgid, &message, sizeof(message.buff), message.mtype, 0);

    if (n > 0) {
        printf("Сервер: получено %zd байт: ", n);
        fflush(stdout);
        if (write(1, message.buff, n) != n) { printf("Ошибка вывода\n"); exit(1); }
    } else {
        perror("Ошибка чтения сообщения");
        exit(1);
    }
    exit(0);
}
