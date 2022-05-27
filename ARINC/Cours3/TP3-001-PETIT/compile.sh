##heptc –target c externc.epi

heptc -target c gnc.ept

cd gnc_c/
cp ../externc_types.h .
cp ../externc.h .
sudo gcc -I `heptc -where`/c gnc.c -c
gcc -I `heptc -where`/c gnc_types.c -c
cd ../

gcc -I `heptc -where`/c externc.c -c
gcc -I `heptc -where`/c externc.o gnc_c/gnc.o gnc_c/gnc_types.o main.c -o main

