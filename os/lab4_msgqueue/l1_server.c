/*
 * l1_server.c — Уровень 1 (A): разбор скелета Server.c
 *
 * Сервер получает ключ через ftok, создаёт очередь (IPC_CREAT),
 * ждёт ОДНО сообщение типа 1 и печатает его. Очередь удаляет клиент.
 *
 * Исправления относительно скелета:
 *  - добавлен #include "mesg.h" (без него нет типа Message);
 *  - в msgrcv размер — sizeof(message.buff), а не sizeof(message):
 *    размер считается БЕЗ поля mtype, иначе ядро может записать
 *    на sizeof(long) байт больше, чем помещается в buff.
 */
#include "mesg.h"

int main(void)
{
    Message message;
    key_t key;
    int msgid;
    ssize_t n;

    /* Получим ключ */
    if ((key = ftok(KEY_PATH, 'A')) < 0) { perror("Невозможно получить ключ"); exit(1); }

    /* Тип принимаемых сообщений */
    message.mtype = 1L;

    /* Создадим очередь сообщений (или откроем существующую) */
    if ((msgid = msgget(key, PERM | IPC_CREAT)) < 0) { perror("Невозможно создать очередь"); exit(1); }
    printf("Сервер: ключ = 0x%x, msgid = %d, жду сообщение типа 1...\n", (unsigned)key, msgid);
    fflush(stdout);

    /* Примем сообщение: блокируемся, пока в очереди нет сообщения типа 1 */
    n = msgrcv(msgid, &message, sizeof(message.buff), message.mtype, 0);

    /* Если сообщение поступило, выведем его содержимое на терминал */
    if (n > 0) {
        printf("Сервер: получено %zd байт: ", n);
        fflush(stdout);
        if (write(1, message.buff, n) != n) { printf("Ошибка вывода\n"); exit(1); }
    } else {
        perror("Ошибка чтения сообщения");
        exit(1);
    }
    /* Удалить очередь должен клиент */
    exit(0);
}
