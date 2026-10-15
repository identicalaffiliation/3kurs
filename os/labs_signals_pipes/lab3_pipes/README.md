# Лабораторная: каналы (pipe и FIFO)

Сборка и полный прогон всех заданий (Linux и macOS):

    ./run.sh

| Задание | Файлы |
|---|---|
| Уровень 1 (A): разбор скелета | `pipe_simple.c` (`input.txt` → `output.txt`) |
| Уровни 2, 3 (A): клиент-сервер на двух pipe | `pipe_cs.c`, `common.h` |
| Уровень 1 (B): FIFO «Здравствуй, Мир!» | `hello_server.c`, `hello_client.c` |
| Уровень 2 (B): неродственные процессы через FIFO | `server_fifo1.c`, `client_fifo1.c` |
| Уровень 3 (B): структурированные сообщения | `server_fifomsg.c`, `client_fifomsg.c`, `mesg.h` |

Ручной запуск FIFO-заданий: в одном терминале `./server_fifo1`,
в другом `./client_fifo1` и ввести полный путь к файлу.
`run_log.txt` — вывод прогона на Linux.

Вместо `mknod(..., S_IFIFO | 0666, 0)` используется `mkfifo(..., 0666)`:
это то же самое, но в macOS `mknod` требует прав root.
