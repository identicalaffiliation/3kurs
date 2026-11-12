/*
 * l2_client.c — Уровень 2 (A): клиент очереди сообщений
 *
 * Получает доступ к очереди сервера, посылает запрос (тип 1),
 * ждёт ответ и печатает его.
 * Запуск: ./l2_client "текст"   (без аргумента — текст по умолчанию)
 */
#include "l2_common.h"

int main(int argc, char **argv)
{
    key_t key;
    int msgid;
    Request req, reply;

    if ((key = ftok(KEY_PATH, 'B')) < 0) { perror("ftok"); exit(1); }
    if ((msgid = msgget(key, 0)) < 0) {
        perror("Клиент: нет доступа к очереди (сервер запущен?)");
        exit(1);
    }

    req.mtype = REQ_TYPE;
    req.pid = getpid();
    if (argc > 1) snprintf(req.text, MAXBUFF, "%s", argv[1]);
    else          snprintf(req.text, MAXBUFF, "Hello from %d", (int)getpid());

    if (msgsnd(msgid, &req, REQ_SIZE, 0) < 0) { perror("Клиент: msgsnd"); exit(1); }

    if (msgrcv(msgid, &reply, REQ_SIZE, REPLY_TYPE(getpid()), 0) < 0) {
        perror("Клиент: msgrcv");
        exit(1);
    }
    printf("Клиент pid %d: получен ответ (тип %ld): %s\n", (int)getpid(), reply.mtype, reply.text);
    return 0;
}
