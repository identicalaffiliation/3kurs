/*
 * mesg.h — общий заголовок для уровней 1 и 2 (очереди сообщений System V)
 */
#ifndef MESG_H
#define MESG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAXBUFF 80
#define PERM    0666

/* Файл, по которому ftok строит ключ. Он должен существовать и не
   пересоздаваться во время работы (иначе сменится номер inode и ключ).
   В методичке — "server" (исполняемый файл), здесь — сам mesg.h:
   программы запускаются из папки лабы. */
#define KEY_PATH "mesg.h"

/* Структура сообщения: первым полем ОБЯЗАТЕЛЬНО идёт long mtype,
   дальше — произвольные данные. */
typedef struct our_msgbuf {
    long mtype;
    char buff[MAXBUFF];
} Message;

#endif
