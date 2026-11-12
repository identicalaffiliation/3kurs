#!/bin/bash
# Сборка и демонстрационный прогон лабораторной «Файловая система»
cd "$(dirname "$0")"
make -s || exit 1
F=list.dat

echo "############ 1. Маленький список: два процесса по 3 элемента ############"
./flist init $F
./flist add $F A 3 &
./flist add $F B 3 &
wait
./flist print $F

echo
echo "############ 2. Два процесса по 3000 элементов С блокировкой ############"
./flist init $F
./flist add $F A 3000 &
./flist add $F B 3000 &
wait
./flist print $F -q

echo
echo "############ 3. То же БЕЗ блокировки (-n) ############"
./flist init $F
./flist add $F A 3000 -n &
./flist add $F B 3000 -n &
wait
./flist print $F -q
echo "      (добавлялось 6000; узлов в файле меньше — процессы писали узлы по одному"
echo "       и тому же смещению и затирали друг друга)"

echo
echo "############ 4. F_GETLK и F_SETLKW: процесс ждёт освобождения ############"
./flist init $F
./flist hold $F 2 &
sleep 0.3
./flist check $F
START=$(date +%s)
./flist add $F C 1
echo "add ждал блокировку примерно $(( $(date +%s) - START )) с"
wait
./flist check $F
ls -l $F
