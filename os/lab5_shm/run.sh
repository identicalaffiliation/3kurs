#!/bin/bash
# Сборка и демонстрационный прогон лабораторной «Разделяемая память»
cd "$(dirname "$0")"
make -s || exit 1

echo "############ Уровень 1 (A): System V shm + семафоры ############"
./l1_server &
SP=$!
sleep 0.3
echo ">>> ipcs -m -s (сегмент и семафоры созданы сервером):"
ipcs -m -s | grep -v "^$" | sed 's/^/    /'
./l1_client
wait $SP

echo
echo "############ Уровень 2 (A): mmap + именованный семафор ############"
rm -f filedata
./l2_mmap 10

echo
echo "############ Уровень 3 (A): файловый клиент-сервер через shm ############"
./l3_server &
SP=$!
sleep 0.3
echo "--- существующий файл ---"
echo "$PWD/shmem.h" | ./l3_client | head -6; sleep 0.1
echo "    ... (первые 6 строк)"
echo "--- несуществующий файл ---"
echo "/no/such/file" | ./l3_client; sleep 0.1
echo "--- клиент убит посреди передачи (kill -9) — сервер не зависает ---"
seq 1 200000 > huge.txt
(echo "$PWD/huge.txt" | ./l3_client > /dev/null 2>&1) &
CP=$!
sleep 0.05; pkill -9 -f "^./l3_client" 2>/dev/null; wait $CP 2>/dev/null
sleep 0.1
echo "$PWD/shmem.h" | ./l3_client 2>/dev/null | head -2
rm -f huge.txt
echo "--- 4 клиента одновременно, файл 60 КБ (порции по 1 КБ) ---"
seq 1 12000 > big.txt
for i in 1 2 3 4; do (echo "$PWD/big.txt" | ./l3_client > copy$i.txt 2>/dev/null) & done
wait $(jobs -p | grep -v "^$SP$") 2>/dev/null
for i in 1 2 3 4; do cmp -s big.txt copy$i.txt && echo "клиент $i: файл получен без искажений"; done
rm -f big.txt copy?.txt
echo ">>> kill -INT $SP"
kill -INT $SP
wait $SP
echo ">>> объекты IPC после остановки сервера:"
ipcs -m -s | grep -v "^$" | sed 's/^/    /'
