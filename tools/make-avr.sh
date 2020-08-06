#!/bin/bash

# Building GCC requires GMP 4.2+, MPFR 3.1.0+ and MPC 0.8.0+
# sudo apt install libgmp-dev libgmp10 libmpfr6 libmpfr-dev libmpc-dev libmpc3 libzstd1 libzstd-dev libisl-dev libisl22 flex texinfo

export PREFIX=`pwd`/avr-install

wget https://mirrors.ocf.berkeley.edu/gnu/binutils/binutils-2.35.tar.xz
tar xJf binutils-2.35.tar.xz
cd binutils-*/
mkdir -p obj-avr
cd obj-avr
../configure --prefix=$PREFIX --target=avr --disable-nls >& logConfigure
make -j8 >& logMake
make install >& logInstall
cd ../..

export PATH=$PREFIX/bin:$PATH

wget http://mirrors.concertpass.com/gcc/releases/gcc-10.2.0/gcc-10.2.0.tar.xz
tar xJf gcc-10.2.0.tar.xz
cd gcc-*/
./contrib/download_prerequisites
mkdir -p obj-avr
cd obj-avr
../configure --prefix=$PREFIX --target=avr --enable-languages=c,c++ \
    --disable-nls --disable-libssp --with-dwarf2 >& logConfigure
make -j8 >& logMake
make install >& logInstall
cd ../..

wget http://download.savannah.gnu.org/releases/avr-libc/avr-libc-2.0.0.tar.bz2
tar xjf avr-libc-2.0.0.tar.bz2
cd avr-libc-2.0.0/
./configure --prefix=$PREFIX --build=`./config.guess` --host=avr >& logConfigure
make -j8 >& logMake
make install >& logInstall
