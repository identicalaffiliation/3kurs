#!/bin/bash
# Сборка и демонстрационный прогон лабораторной «Очереди сообщений»
cd "$(dirname "$0")"
make -s || exit 1

echo "############ Уровень 1 (A): скелет server/client ############"
./l1_server &
SP=$!
sleep 0.3
echo ">>> ipcs -q (очередь создана сервером):"
ipcs -q | sed 's/^/    /'
./l1_client
wait $SP

echo
echo "############ Уровень 2 (A): сервер + 5 клиентов одновременно ############"
./l2_server &
SP=$!
sleep 0.3
for i in 1 2 3 4 5; do ./l2_client "запрос клиента $i" & done
wait $(jobs -p | grep -v "^$SP$") 2>/dev/null
sleep 0.2
echo ">>> kill -INT $SP"
kill -INT $SP
wait $SP
echo ">>> очередь после остановки сервера:"
ipcs -q | sed 's/^/    /'

echo
echo "############ Уровень 3 (A): файловый сервер на двух очередях ############"
./l3_server_msg &
SP=$!
sleep 0.3
echo "$PWD/l3_msg.h" | ./l3_client_msg | head -8
echo "    ... (выведены первые 8 строк)"
wait $SP
echo "--- несуществующий файл ---"
./l3_server_msg > /dev/null &
SP=$!
sleep 0.3
echo "/no/such/file" | ./l3_client_msg
wait $SP
echo "--- проверка: файл передан без искажений ---"
seq 1 2000 > big.txt
./l3_server_msg > /dev/null &
SP=$!
sleep 0.3
echo "$PWD/big.txt" | ./l3_client_msg > big_copy.txt 2>/dev/null
wait $SP
cmp big.txt big_copy.txt && echo "big.txt ($(wc -l < big.txt | tr -d ' ') строк) передан без искажений"
rm -f big.txt big_copy.txt
