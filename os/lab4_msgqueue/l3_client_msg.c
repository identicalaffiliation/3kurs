#include "l3_msg.h"

static void client(int readid, int writeid);

int main(void)
{
    int readid, writeid;

    if ((writeid = msgget(MQ_KEY1, 0666)) < 0) { perror("Client: can not get writeid"); exit(1); }
    fprintf(stderr, "Client: writeid=%d\n", writeid);
    if ((readid = msgget(MQ_KEY2, 0666)) < 0) { perror("Client: can not get readid"); exit(1); }
    fprintf(stderr, "Client: readid=%d\n", readid);

    client(readid, writeid);

    if (msgctl(readid, IPC_RMID, NULL) < 0)  { perror("Client: can not delete message queue2"); exit(1); }
    if (msgctl(writeid, IPC_RMID, NULL) < 0) { perror("Client: can not delete message queue1"); exit(1); }
    fprintf(stderr, "Client: обе очереди удалены\n");
    exit(0);
}

static void client(int readid, int writeid)
{
    size_t len;
    ssize_t n;
    struct mymesg ourmesg;

    fprintf(stderr, "Client: readid=%d writeid=%d\n", readid, writeid);
    fprintf(stderr, "Client: введите полное имя файла: ");
    if (fgets(ourmesg.mesg_data, MAXMESGDATA, stdin) == NULL) return;
    len = strlen(ourmesg.mesg_data);
    if (len > 0 && ourmesg.mesg_data[len - 1] == '\n') len--;
    ourmesg.mesg_len = len;
    ourmesg.mesg_type = 1;
    fprintf(stderr, "%.*s\n", (int)len, ourmesg.mesg_data);

    mesg_send(writeid, &ourmesg);
    fprintf(stderr, "Client: before recv!\n");

    while ((n = mesg_recv(readid, &ourmesg)) > 0)
        if (write(1, ourmesg.mesg_data, n) != n) { perror("write"); return; }
    if (n < 0) perror("Client: mesg_recv");
    else fprintf(stderr, "Client: получено пустое сообщение — конец файла\n");
}
