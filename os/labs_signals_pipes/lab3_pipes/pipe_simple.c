#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int filedes[2];                   
    char temp_ch;

    setvbuf(stdout, NULL, _IONBF, 0);
    if (pipe(filedes) < 0) {
        printf("Father can not create Pipe!\n");
        exit(1);
    }
    printf("Father created Pipe! fd[0]=%d (чтение), fd[1]=%d (запись)\n", filedes[0], filedes[1]);

    if (fork() == 0) {                      
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

    close(filedes[1]);
    printf("Father is working again!\n");
    FILE *fout = fopen("output.txt", "w");
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
