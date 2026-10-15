/*
 * pipe_cs.c — Уровень 2, 3 (A): клиент-сервер на двух неименованных каналах
 *
 *   родитель = клиент, сын = сервер
 *   pipe1: клиент -> сервер (имя файла)
 *   pipe2: сервер -> клиент (содержимое файла или сообщение об ошибке)
 *
 * Запуск:  echo /etc/hostname | ./pipe_cs
 *     или  ./pipe_cs   и ввести путь с клавиатуры
 */
#include <sys/wait.h>
#include "common.h"

int main(void)
{
    int pipe1[2], pipe2[2];
    pid_t childpid;

    if (pipe(pipe1) < 0 || pipe(pipe2) < 0) { perror("pipe"); exit(1); }

    if ((childpid = fork()) == 0) {          /* child = сервер */
        close(pipe1[1]);
        close(pipe2[0]);
        server(pipe1[0], pipe2[1]);
        exit(0);
    }
    /* parent = клиент */
    close(pipe1[0]);
    close(pipe2[1]);

    client(pipe2[0], pipe1[1]);

    waitpid(childpid, NULL, 0);              /* ждём завершения сервера */
    return 0;
}
