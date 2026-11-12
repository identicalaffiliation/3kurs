/*
 * l1_client.c — Уровень 1 (A): разбор скелета Client.c
 *
 * Клиент получает тот же ключ (тот же файл и тот же 'A'), открывает
 * уже созданную сервером очередь (флаги 0), посылает сообщение типа 1
 * и удаляет очередь (msgctl IPC_RMID).
 *
 * Исправление: в скелете клиент удаляет очередь СРАЗУ после msgsnd.
 * Если сервер ещё не успел забрать сообщение, оно пропадёт вместе
 * с очередью (гонка). Здесь клиент перед удалением ждёт, пока очередь
 * опустеет: msgctl(IPC_STAT) возвращает число сообщений msg_qnum.
 */
#include "mesg.h"

int main(void)
{
    Message message;
    key_t key;
    int msgid, length;
    struct msqid_ds st;

    /* Тип посылаемого сообщения */
    message.mtype = 1L;

    /* Получим ключ */
    if ((key = ftok(KEY_PATH, 'A')) < 0) { perror("Невозможно получить ключ"); exit(1); }

    /* Получим доступ к очереди сообщений; очередь уже должна быть создана сервером */
    if ((msgid = msgget(key, 0)) < 0) { perror("Невозможно получить доступ к очереди"); exit(1); }

    /* Подготовим сообщение */
    if ((length = snprintf(message.buff, MAXBUFF, "IPC Messages!\n")) < 0) {
        printf("Ошибка копирования в буфер\n"); exit(1);
    }

    /* Передадим сообщение: length — длина ДАННЫХ, без поля mtype */
    if (msgsnd(msgid, (void *)&message, length, 0) != 0) {
        perror("Ошибка записи сообщения в очередь"); exit(1);
    }
    printf("Клиент: отправлено %d байт, тип 1\n", length);

    /* Ждём, пока сервер заберёт сообщение (msg_qnum == 0) */
    do {
        if (msgctl(msgid, IPC_STAT, &st) < 0) { perror("IPC_STAT"); exit(1); }
        if (st.msg_qnum > 0) usleep(10000);
    } while (st.msg_qnum > 0);
    printf("Клиент: сообщение забрал процесс pid %d (msg_lrpid)\n", (int)st.msg_lrpid);

    /* Удалим очередь сообщений */
    if (msgctl(msgid, IPC_RMID, 0) < 0) { perror("Ошибка удаления очереди"); exit(1); }
    printf("Клиент: очередь удалена\n");
    exit(0);
}
