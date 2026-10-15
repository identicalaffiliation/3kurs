#!/bin/bash
# Сборка и демонстрационный прогон всех заданий лабораторной «Каналы»
cd "$(dirname "$0")"
make -s || exit 1
rm -f /tmp/fifo.1 /tmp/fifo.2 /tmp/fifo.hello

echo "############ 1 (A). pipe_simple: сын -> отец через pipe ############"
./pipe_simple
echo "--- output.txt совпадает с input.txt? ---"
cmp input.txt output.txt && echo "да, файлы идентичны"

echo
echo "############ 2,3 (A). pipe_cs: клиент-сервер на двух pipe ############"
echo "--- существующий файл ---"
echo "$PWD/input.txt" | ./pipe_cs
echo "--- несуществующий файл ---"
echo "/no/such/file.txt" | ./pipe_cs

echo
echo "############ 1 (B). FIFO «Здравствуй, Мир!» ############"
./hello_server &
sleep 0.3
ls -l /tmp/fifo.hello      # первый символ p — это FIFO
./hello_client
wait

echo
echo "############ 2 (B). Неродственные клиент и сервер на FIFO ############"
./server_fifo1 &
sleep 0.3
echo "$PWD/input.txt" | ./client_fifo1
wait
echo "--- ошибка ---"
./server_fifo1 &
sleep 0.3
echo "/no/such/file.txt" | ./client_fifo1
wait

echo
echo "############ 3 (B). Структурированные сообщения через FIFO ############"
./server_fifomsg &
sleep 0.3
echo "$PWD/input.txt" | ./client_fifomsg
wait
echo "--- ошибка ---"
./server_fifomsg &
sleep 0.3
echo "/no/such/file.txt" | ./client_fifomsg
wait
echo "--- большой файл (несколько сообщений DATA) ---"
seq 1 3000 > big.txt
./server_fifomsg > /dev/null &
sleep 0.3
echo "$PWD/big.txt" | ./client_fifomsg > big_copy.txt
wait
cmp big.txt big_copy.txt && echo "big.txt ($(wc -c < big.txt | tr -d " ") байт) передан без искажений"
rm -f big.txt big_copy.txt
