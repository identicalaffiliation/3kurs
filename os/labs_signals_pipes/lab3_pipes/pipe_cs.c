#include <sys/wait.h>
#include "common.h"

int main(void)
{
    int pipe1[2], pipe2[2];
    pid_t childpid;

    if (pipe(pipe1) < 0 || pipe(pipe2) < 0) { perror("pipe"); exit(1); }

    if ((childpid = fork()) == 0) {        
        close(pipe1[1]);
        close(pipe2[0]);
        server(pipe1[0], pipe2[1]);
        exit(0);
    }
    /* parent = клиент */
    close(pipe1[0]);
    close(pipe2[1]);

    client(pipe2[0], pipe1[1]);

    waitpid(childpid, NULL, 0);         
    return 0;
}
