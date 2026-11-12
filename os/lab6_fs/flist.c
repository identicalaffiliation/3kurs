/*
 * flist.c — Лабораторная «Файловая система», уровни 1, 2, 3 (A)
 *
 * Задание: два процесса заносят очередной элемент в связанный список,
 * который хранится в файле.
 *
 * Формат файла (все числа — int32_t):
 *   [ заголовок: magic | head | tail | count ]  смещение 0
 *   [ узел: value | pid | author[8] | next ]   дописываются в конец файла
 * head/tail/next — СМЕЩЕНИЯ узлов в файле (вместо указателей), -1 = нет.
 *
 * Добавление элемента (ничего не стирается, узлы только дописываются):
 *   1) прочитать заголовок                       lseek(0) + read
 *   2) дописать новый узел в конец файла         lseek(SEEK_END) + write
 *   3) прописать его смещение в next старого хвоста (или в head)
 *   4) обновить tail и count в заголовке         lseek(0) + write
 * Это критическая секция: если два процесса выполнят её одновременно,
 * оба прочитают один и тот же tail, и один из новых узлов «потеряется».
 * Поэтому на время шагов 1–4 файл блокируется: fcntl(F_SETLKW).
 *
 * Команды:
 *   ./flist init  FILE                 создать пустой список (creat)
 *   ./flist add   FILE NAME N [-n]     добавить N элементов от имени NAME;
 *                                      -n — БЕЗ блокировки (покажет потери)
 *   ./flist print FILE [-q]            обойти список и проверить его;
 *                                      -q — только итог, без узлов
 *   ./flist hold  FILE SEC             захватить блокировку на SEC секунд
 *   ./flist check FILE                 F_GETLK: кто держит блокировку; dup
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAGIC 0x4C495354            /* "LIST" */

typedef struct {
    int32_t magic, head, tail, count;
} Header;

typedef struct {
    int32_t value;
    int32_t pid;
    char    author[8];
    int32_t next;
} Node;

static void die(const char *what) { perror(what); exit(1); }

/* Прочитать/записать ровно n байт по смещению off (lseek + read/write) */
static void read_at(int fd, off_t off, void *buf, size_t n)
{
    if (lseek(fd, off, SEEK_SET) < 0) die("lseek");
    if (read(fd, buf, n) != (ssize_t)n) die("read");
}
static void write_at(int fd, off_t off, const void *buf, size_t n)
{
    if (lseek(fd, off, SEEK_SET) < 0) die("lseek");
    if (write(fd, buf, n) != (ssize_t)n) die("write");
}

/* Блокировка всего файла: type = F_WRLCK (захват) или F_UNLCK (снятие).
   F_SETLKW — если файл занят, процесс засыпает до освобождения. */
static void lock_file(int fd, short type)
{
    struct flock fl;
    memset(&fl, 0, sizeof fl);
    fl.l_type = type;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;                   /* 0 = до конца файла, т.е. весь файл */
    while (fcntl(fd, F_SETLKW, &fl) < 0) {
        if (errno != EINTR) die("fcntl F_SETLKW");
    }
}

static int open_list(const char *path, int flags)
{
    int fd = open(path, flags);
    if (fd < 0) die(path);
    Header h;
    read_at(fd, 0, &h, sizeof h);
    if (h.magic != MAGIC) { fprintf(stderr, "%s: не файл списка\n", path); exit(1); }
    return fd;
}

static void cmd_init(const char *path)
{
    /* creat = open(path, O_WRONLY | O_CREAT | O_TRUNC, mode) */
    int fd = creat(path, 0644);
    if (fd < 0) die("creat");
    Header h = { MAGIC, -1, -1, 0 };
    write_at(fd, 0, &h, sizeof h);
    if (close(fd) < 0) die("close");
    printf("init: создан пустой список %s (%zu байт заголовка)\n", path, sizeof h);
}

static void add_one(int fd, const char *name, int value, int use_lock)
{
    Header h;
    Node n;
    off_t off;

    if (use_lock) lock_file(fd, F_WRLCK);

    read_at(fd, 0, &h, sizeof h);                       /* 1 */

    memset(&n, 0, sizeof n);                            /* 2 */
    n.value = value;
    n.pid = (int32_t)getpid();
    strncpy(n.author, name, sizeof n.author - 1);
    n.next = -1;
    if ((off = lseek(fd, 0, SEEK_END)) < 0) die("lseek");
    if (write(fd, &n, sizeof n) != (ssize_t)sizeof n) die("write");

    if (h.tail < 0)                                     /* 3 */
        h.head = (int32_t)off;
    else {
        int32_t o = (int32_t)off;
        write_at(fd, h.tail + offsetof(Node, next), &o, sizeof o);
    }

    h.tail = (int32_t)off;                              /* 4 */
    h.count++;
    write_at(fd, 0, &h, sizeof h);

    if (use_lock) lock_file(fd, F_UNLCK);
}

static void cmd_add(const char *path, const char *name, int count, int use_lock)
{
    int fd = open_list(path, O_RDWR);
    for (int i = 1; i <= count; i++)
        add_one(fd, name, i, use_lock);
    close(fd);
    printf("add: процесс %s (pid %d) добавил элементов: %d, %s\n",
           name, (int)getpid(), count, use_lock ? "с блокировкой" : "БЕЗ блокировки");
}

static void cmd_print(const char *path, int quiet)
{
    int fd = open_list(path, O_RDONLY);
    Header h;
    Node n;
    struct stat st;
    int reachable = 0, shown = 0;
    char names[16][8];
    int per[16] = {0}, nnames = 0;

    read_at(fd, 0, &h, sizeof h);
    if (fstat(fd, &st) < 0) die("fstat");
    long stored = (long)(st.st_size - (off_t)sizeof h) / (long)sizeof(Node);

    int cycle = 0, broken = 0;
    for (int32_t off = h.head; off >= 0; off = n.next) {
        if (reachable >= stored) { cycle = 1; break; }     /* узлов больше, чем есть в файле */
        if (off + (off_t)sizeof n > st.st_size) { broken = 1; break; }
        read_at(fd, off, &n, sizeof n);
        reachable++;
        int k;
        for (k = 0; k < nnames && strncmp(names[k], n.author, 8) != 0; k++) ;
        if (k == nnames && nnames < 16) memcpy(names[nnames++], n.author, 8);
        if (k < 16) per[k]++;
        if (!quiet && shown < 40) {
            printf("  @%-6d %-6.8s value=%-5d pid=%-6d next=%d\n",
                   off, n.author, n.value, n.pid, n.next);
            shown++;
        }
    }
    if (!quiet && reachable > shown) printf("  ... ещё %d узлов\n", reachable - shown);

    printf("Итог: в заголовке count = %d, узлов в файле = %ld, достижимо по списку = %d\n",
           h.count, stored, reachable);
    for (int k = 0; k < nnames; k++) printf("      от %-6.8s в списке элементов: %d\n", names[k], per[k]);
    if (reachable == stored && reachable == h.count && !cycle && !broken)
        printf("      Список целый: ни один элемент не потерян.\n");
    else if (cycle)
        printf("      СПИСОК ИСПОРЧЕН: в нём образовался цикл (next указывает на уже пройденный узел).\n");
    else if (broken)
        printf("      СПИСОК ИСПОРЧЕН: ссылка next указывает за конец файла.\n");
    else
        printf("      СПИСОК ИСПОРЧЕН: %ld узлов лежат в файле, но на них никто не ссылается;\n"
               "      count в заголовке не совпадает с реальным числом узлов.\n",
               stored - reachable);
    close(fd);
}

static void cmd_hold(const char *path, int sec)
{
    int fd = open_list(path, O_RDWR);
    lock_file(fd, F_WRLCK);
    printf("hold: pid %d держит блокировку %s %d с\n", (int)getpid(), path, sec);
    fflush(stdout);
    sleep(sec);
    lock_file(fd, F_UNLCK);
    close(fd);
    printf("hold: блокировка снята\n");
}

static void cmd_check(const char *path)
{
    int fd = open_list(path, O_RDWR);
    struct flock fl;
    memset(&fl, 0, sizeof fl);
    fl.l_type = F_WRLCK;            /* «могу ли я поставить такую блокировку?» */
    fl.l_whence = SEEK_SET;
    if (fcntl(fd, F_GETLK, &fl) < 0) die("fcntl F_GETLK");
    if (fl.l_type == F_UNLCK)
        printf("check: F_GETLK — файл свободен\n");
    else
        printf("check: F_GETLK — файл заблокирован процессом pid %d (%s)\n",
               (int)fl.l_pid, fl.l_type == F_WRLCK ? "блокировка записи" : "блокировка чтения");

    /* dup: новый дескриптор ссылается на тот же открытый файл,
       поэтому у них ОБЩИЙ указатель смещения */
    int fd2 = dup(fd);
    if (fd2 < 0) die("dup");
    lseek(fd, 8, SEEK_SET);
    printf("check: dup(%d) = %d; после lseek(fd, 8) смещение fd2 = %ld (общее)\n",
           fd, fd2, (long)lseek(fd2, 0, SEEK_CUR));
    close(fd2);
    close(fd);
}

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    if (argc >= 3 && strcmp(argv[1], "init") == 0) cmd_init(argv[2]);
    else if (argc >= 5 && strcmp(argv[1], "add") == 0)
        cmd_add(argv[2], argv[3], atoi(argv[4]), !(argc > 5 && strcmp(argv[5], "-n") == 0));
    else if (argc >= 3 && strcmp(argv[1], "print") == 0)
        cmd_print(argv[2], argc > 3 && strcmp(argv[3], "-q") == 0);
    else if (argc >= 4 && strcmp(argv[1], "hold") == 0) cmd_hold(argv[2], atoi(argv[3]));
    else if (argc >= 3 && strcmp(argv[1], "check") == 0) cmd_check(argv[2]);
    else {
        fprintf(stderr,
            "Использование:\n"
            "  %s init  FILE\n"
            "  %s add   FILE NAME N [-n]   (-n — без блокировки)\n"
            "  %s print FILE [-q]\n"
            "  %s hold  FILE SEC\n"
            "  %s check FILE\n", argv[0], argv[0], argv[0], argv[0], argv[0]);
        return 1;
    }
    return 0;
}
