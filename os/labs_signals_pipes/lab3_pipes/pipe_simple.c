/*
 * pipe_simple.c — Уровень 1 (A): разбор скелета из методички
 *
 * Что происходит:
 *  1. Отец создаёт канал pipe(filedes): filedes[0] — чтение, filedes[1] — запись.
 *  2. fork(): сын наследует ОБА дескриптора канала.
 *  3. Сын закрывает ненужный конец для чтения, посимвольно читает input.txt
 *     и пишет символы в канал, затем закрывает конец для записи.
 *  4. Отец закрывает конец для записи (иначе read никогда не вернёт 0 —
 *     канал будет считаться открытым на запись им самим) и читает из канала,
 *     пока read не вернёт 0 (все писатели закрыли канал). Прочитанное
 *     печатается на экран и пишется в output.txt.
 *
 * Исправления относительно скелета: недостающие #include, проверка fopen,
 * "%c" в fscanf пропускал бы ничего — заменено на fgetc (читает и пробелы,
 * и переводы строк), ожидание сына через wait.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int filedes[2];                          /* 0 - read; 1 - write */
    char temp_ch;

    setvbuf(stdout, NULL, _IONBF, 0);
    if (pipe(filedes) < 0) {
        printf("Father can not create Pipe!\n");
        exit(1);
    }
    printf("Father created Pipe! fd[0]=%d (чтение), fd[1]=%d (запись)\n", filedes[0], filedes[1]);

    if (fork() == 0) {                       /* ---- сын: писатель ---- */
        int ch;
        close(filedes[0]);
        printf("Child is working!\n");
        FILE *fin = fopen("input.txt", "r");
        if (!fin) { perror("input.txt"); exit(1); }
        while ((ch = fgetc(fin)) != EOF) {
            char c = (char)ch;
            if (write(filedes[1], &c, 1) != 1) { perror("write"); break; }
        }
        fclose(fin);
        close(filedes[1]);
        printf("End of Child!\n");
        exit(0);
    }

    /* ---- отец: читатель ---- */
    close(filedes[1]);
    printf("Father is working again!\n");
    FILE *fout = fopen("output.txt", "w");
    /* читаем побайтно до конца канала; на экран выводим уже после чтения,
       чтобы сообщения сына не перемешивались с данными */
    char *data = NULL;
    size_t len = 0, cap = 0;
    while (read(filedes[0], &temp_ch, 1) > 0) {
        fputc(temp_ch, fout);
        if (len == cap) { cap = cap ? cap * 2 : 256; data = realloc(data, cap); }
        data[len++] = temp_ch;
    }
    fclose(fout);
    wait(NULL);
    printf("---- данные из канала (%zu байт) ----\n%.*s", len, (int)len, data);
    printf("---- конец данных (read вернул 0) ----\n");
    free(data);
    close(filedes[0]);
    printf("End of Father! Данные сохранены в output.txt\n");
    return 0;
}
