#include "l3_msg.h"

static void server(int readid, int writeid);

int main(void)
{
    int readid, writeid;

    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Server: HELLO! pid=%d\n", getpid());

    if ((readid = msgget(MQ_KEY1, 0666 | IPC_CREAT)) < 0) { perror("Server: can not get readid"); exit(1); }
    printf("Server: readid=%d\n", readid);
    if ((writeid = msgget(MQ_KEY2, 0666 | IPC_CREAT)) < 0) { perror("Server: can not get writeid"); exit(1); }
    printf("Server: writeid=%d\n", writeid);

    server(readid, writeid);
    exit(0);
}

static void server(int readid, int writeid)
{
    FILE *fp;
    ssize_t n;
    struct mymesg ourmesg;
    int lines = 0;

    printf("Server: readid=%d writeid=%d\n", readid, writeid);

    ourmesg.mesg_type = 1;
    if ((n = mesg_recv(readid, &ourmesg)) <= 0) { perror("Server: can not read file name"); exit(1); }
    ourmesg.mesg_data[n] = '\0';
    printf("Server: file name %s\n", ourmesg.mesg_data);

    if ((fp = fopen(ourmesg.mesg_data, "r")) == NULL) {
        char name[MAXMESGDATA];
        snprintf(name, sizeof name, "%s", ourmesg.mesg_data);
        printf("Server: can not open file name\n");
        ourmesg.mesg_len = snprintf(ourmesg.mesg_data, MAXMESGDATA,
                                    "Server: невозможно открыть %s: %s\n", name, strerror(errno));
        mesg_send(writeid, &ourmesg);
    } else {
        printf("Server: %s is opened\n", ourmesg.mesg_data);
        while (fgets(ourmesg.mesg_data, MAXMESGDATA, fp) != NULL) {
            ourmesg.mesg_len = strlen(ourmesg.mesg_data);
            if (mesg_send(writeid, &ourmesg) < 0) { perror("Server: mesg_send"); break; }
            lines++;
        }
        fclose(fp);
        printf("Server: отправлено строк: %d\n", lines);
    }
    ourmesg.mesg_len = 0;                      
    mesg_send(writeid, &ourmesg);
    printf("Server: отправлен признак конца (сообщение длины 0)\n");
}
