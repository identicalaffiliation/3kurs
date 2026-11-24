#include <signal.h>
#include "l2_common.h"

static int msgid = -1;
static struct sigaction old_int;          

static void sigint_handler(int sig)
{
    char msg[96];
    int len;
    (void)sig;
    if (msgid >= 0 && msgctl(msgid, IPC_RMID, NULL) == 0)
        len = snprintf(msg, sizeof msg, "\nСервер: получен SIGINT, очередь %d удалена\n", msgid);
    else
        len = snprintf(msg, sizeof msg, "\nСервер: получен SIGINT, очередь удалить не удалось\n");
    if (write(STDOUT_FILENO, msg, len) < 0) {}
    sigaction(SIGINT, &old_int, NULL);    
    _exit(0);
}

int main(void)
{
    key_t key;
    Request req, reply;
    struct sigaction sa;
    int served = 0;

    setvbuf(stdout, NULL, _IONBF, 0);

    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, &old_int);

    if ((key = ftok(KEY_PATH, 'B')) < 0) { perror("ftok"); exit(1); }
    if ((msgid = msgget(key, PERM | IPC_CREAT)) < 0) { perror("msgget"); exit(1); }
    printf("Сервер: pid = %d, ключ = 0x%x, msgid = %d\n", getpid(), (unsigned)key, msgid);
#ifdef REPLY_TYPE_2
    printf("Сервер: ответы имеют тип 2\n");
#else
    printf("Сервер: ответы имеют тип = PID клиента\n");
#endif

    for (;;) {
        ssize_t n = msgrcv(msgid, &req, REQ_SIZE, REQ_TYPE, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("Сервер: msgrcv");
            kill(getpid(), SIGINT);       
        }
        served++;
        printf("Сервер: [%d] запрос от клиента pid %d: \"%s\"\n", served, (int)req.pid, req.text);

        reply.mtype = REPLY_TYPE(req.pid);
        reply.pid = getpid();
        snprintf(reply.text, MAXBUFF, "ответ #%d на \"%.40s\"", served, req.text);
        if (msgsnd(msgid, &reply, REQ_SIZE, 0) < 0) {
            perror("Сервер: msgsnd");
            kill(getpid(), SIGINT);
        }
    }
}
