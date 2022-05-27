#!/bin/bash
heptc -target c complex.ept
heptc -target c complex_io.epi
heptc -target c complexes.ept
heptc -c twiddle.epi
heptc -target c complex_vec_io.epi
heptc -target c bitrev.epi
heptc -target c fft.ept
heptc -target c complex_vec_io.epi
heptc complex_vec_io.epi
heptc -target c ifft.ept
heptc -c sndlib.epi
heptc -c myprog.epi
heptc -c main.ept
heptc -target c main.ept

cp complex_c/complex_types.h complexes_c/.
cp complex_io_types.h complexes_c/.
cp complex_io.h complexes_c/.
cp -r complex_c/ complexes_c/.
cp bitrev_types.h fft_c/.
cp bitrev.h fft_c/.
cp complex_c/complex_types.h fft_c/.
cp -r complex_c/ fft_c/.
cp complex_c/complex_types.h main_c/.
cp complex_vec_io_types.h main_c/.
cp fft_c/fft_types.h main_c/.
cp bitrev_types.h main_c/.
cp bitrev.h main_c/.

cp complex_c/complex_types.h main_c/.
cp complex_c/complex_types.c main_c/.
cp -r complex_c/ main_c/.
cp complex_vec_io.h main_c/.
cp fft_c/fft.h main_c/.
cp complex_vec_io.c main_c/.
cp fft_c/fft.c main_c/.
cp ifft_c/ifft.h main_c/.
cp ifft_c/ifft_types.h main_c/.
cp complex_io_types.h main_c/.
cp complex_io_types.c main_c/.


if [ ! -d "build" ];then
	mkdir build/
	cp `heptc -where`/c/pervasives.h build/.
fi


gcc -std=c99 -g -I$(pwd) -I'build' -c complex_c/complex_types.c complex_c/complex.c -lm -lc -lgcc
gcc -std=c99 -g -I$(pwd) -I'build' -c complex_io_types.c complex_io.c -lm -lc -lgcc
gcc -std=c99 -g -I$(pwd) -I'build' -c complexes_c/complexes_types.c complexes_c/complexes.c -lm -lc -lgcc
gcc -std=c99 -g -I$(pwd) -I'build' -I'complex_c' -c twiddle_init.c twiddle.c -lm -lc -lgcc
gcc -std=c99 -g -I$(pwd) -I'build' -c bitrev.c complex_vec_io.c -lm -lc -lgcc
gcc -std=c99 -g -I$(pwd) -I'build' -I'twiddle_c' -c fft_c/fft.c fft_c/fft_types.c -lm -lc -lgcc
gcc -std=c99 -g -I$(pwd) -I'build' -I'twiddle_c' -I'fft_c' -c ifft_c/ifft.c ifft_c/ifft_types.c -lm -lc -lgcc
gcc -std=c99 -g -I$(pwd) -I'build' -I'bitrev_c' -c sndlib.c myprog.c main_c/main.c -lm -lc -lgcc
gcc -std=c99 -g -I$(pwd) -I'build' -I'complex_c' -I'fft_c' -I'bitrev_c' -I'sndlib_c' -I'myprog_c' -c process.c -lm -lc -lgcc

gcc -std=c99 -g -I'build' *.o -o main -lm -lc -lgcc


rec -t raw -r 44100 -e signed -b 16 -c 2 - | ./main | play -t raw -r 44100 -e signed -b 16 -c 2 -

