#!/bin/bash

# Building GCC requires GMP 4.2+, MPFR 3.1.0+ and MPC 0.8.0+
# sudo apt install libgmp-dev libgmp10 libmpfr6 libmpfr-dev libmpc-dev libmpc3 flex libzstd1 libzstd-dev libisl-dev libisl22

export PREFIX=`pwd`/avr-install

git clone git://sourceware.org/git/binutils-gdb.git binutils-source
cd binutils-source/
git checkout binutils-2_35
mkdir -p obj-avr
cd obj-avr
../configure --prefix=$PREFIX --target=avr --disable-nls
make -j8
make install
cd ../..

export PATH=$PREFIX/bin:$PATH

git clone git://gcc.gnu.org/git/gcc.git gcc-source
cd gcc-source
./contrib/download_prerequisites
git checkout releases/gcc-10.2.0
mkdir -p obj-avr
cd obj-avr
../configure --prefix=$PREFIX --target=avr --enable-languages=c,c++ \
    --disable-nls --disable-libssp --with-dwarf2
make -j8
make install
cd ../..

wget http://download.savannah.gnu.org/releases/avr-libc/avr-libc-2.0.0.tar.bz2
tar xjf avr-libc-2.0.0.tar.bz2
cd avr-libc-2.0.0/
./configure --prefix=$PREFIX --build=`./config.guess` --host=avr
make -j8
make install
