#include "mesg.h"

int main(void)
{
    Message message;
    key_t key;
    int msgid, length;
    struct msqid_ds st;

    message.mtype = 1L;

    if ((key = ftok(KEY_PATH, 'A')) < 0) { perror("Невозможно получить ключ"); exit(1); }

    if ((msgid = msgget(key, 0)) < 0) { perror("Невозможно получить доступ к очереди"); exit(1); }

    if ((length = snprintf(message.buff, MAXBUFF, "IPC Messages!\n")) < 0) {
        printf("Ошибка копирования в буфер\n"); exit(1);
    }

    if (msgsnd(msgid, (void *)&message, length, 0) != 0) {
        perror("Ошибка записи сообщения в очередь"); exit(1);
    }
    printf("Клиент: отправлено %d байт, тип 1\n", length);

    do {
        if (msgctl(msgid, IPC_STAT, &st) < 0) { perror("IPC_STAT"); exit(1); }
        if (st.msg_qnum > 0) usleep(10000);
    } while (st.msg_qnum > 0);
    printf("Клиент: сообщение забрал процесс pid %d (msg_lrpid)\n", (int)st.msg_lrpid);

    if (msgctl(msgid, IPC_RMID, 0) < 0) { perror("Ошибка удаления очереди"); exit(1); }
    printf("Клиент: очередь удалена\n");
    exit(0);
}
