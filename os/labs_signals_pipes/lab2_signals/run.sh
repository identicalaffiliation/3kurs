#!/bin/bash
# Сборка и демонстрационный прогон всех заданий лабораторной «Сигналы»
cd "$(dirname "$0")"
make -s || exit 1

echo "############ 0. kill -l (Уровень 1 A) ############"
kill -l

echo
echo "############ 1. father (Уровень 1 A) ############"
./father
echo "--- ps.txt ---"
cat ps.txt

echo
echo "############ 2. sig_father + sig_son (Уровень 2 A) ############"
./sig_father &
FP=$!
sleep 1
echo ">>> с терминала: kill -s USR2 $FP"; kill -s USR2 $FP; sleep 0.5
echo ">>> с терминала: kill -s USR1 $FP"; kill -s USR1 $FP; sleep 0.5
echo ">>> с терминала: kill -s INT  $FP (действие по умолчанию — завершение)"; kill -s INT $FP
wait $FP; echo "sig_father завершился, код $? (128+2 = SIGINT)"

echo
echo "############ 3. Нити и сигналы (Уровень 3 A) ############"
if [ "$(uname -s)" = Linux ]; then
echo "--- режим 0: kill <TID второй нити> с терминала ---"
./thread_sig 0 > t0.log &
TP=$!
sleep 1; cat t0.log
TID2=$(ls /proc/$TP/task | sort -n | sed -n 2p)
echo ">>> kill $TID2  (TID второй нити)"; kill $TID2
wait $TP; echo "процесс завершён целиком, код $? (128+15 = SIGTERM)"; rm -f t0.log
else
echo "--- режим 0 пропущен: в macOS нельзя послать kill по идентификатору нити ---"
fi
echo "--- режим 1: pthread_kill(t2, SIGUSR1) без обработчика ---"
./thread_sig 1; echo "код завершения процесса: $? (128+10 = SIGUSR1 — погиб весь процесс)"
echo "--- режим 2: свой обработчик + pthread_exit(NULL) ---"
./thread_sig 2

echo
echo "############ 4. sigact (Уровни 1-3 B) ############"
echo "--- вариант 1: USR1 с терминала, затем INT (сон в обработчике сокращён до 3 с) ---"
./sigact 3 &
SP=$!
sleep 0.5
echo ">>> kill -s USR1 $SP"; kill -s USR1 $SP; sleep 1
echo ">>> kill -s INT  $SP (пока работает обработчик — SIGINT заблокирован)"; kill -s INT $SP
if [ -r /proc/$SP/status ]; then
  grep SigBlk /proc/$SP/status | sed 's/^/    маска блокировки: /'
  grep ShdPnd /proc/$SP/status | sed 's/^/    ожидающие сигналы: /'
fi
wait $SP; echo "sigact завершился, код $? — SIGINT доставлен только после выхода из обработчика"

echo "--- вариант 2: обработчик сам вызывает kill(getpid(), SIGINT) ---"
./sigact 3 -k &
SP=$!
sleep 0.5
echo ">>> kill -s USR2 $SP"; kill -s USR2 $SP
wait $SP; echo "sigact завершился, код $?"

echo "--- для сравнения: без сигналов SIGINT -> нормальное завершение ---"
./sigact 1
