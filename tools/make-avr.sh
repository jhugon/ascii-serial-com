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
nice make -j8 >& logMake
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
nice make -j8 >& logMake
make install >& logInstall
cd ../..

## avrdude

git clone https://github.com/jhugon/avrdude.git
cd avrdude/
./bootstrap >& logBootstrap
mkdir -p obj
cd obj
../configure --prefix=$PREFIX >& logConfigure
nice make -j8 >& logMake
make install >& logInstall
cd ../..

######## avr-libc

git clone https://github.com/jhugon/avr-libc.git
cd avr-libc/
./bootstrap >& logBootstrap
cd ..

### Default

mkdir -p avr-libc/obj-avr
cd avr-libc/obj-avr
../configure --prefix=$PREFIX --build=`../config.guess` --host=avr >& logConfigure
nice make -j8 >& logMake
make install >& logInstall
cd ../..

### debug
export PREFIX=`pwd`/avr-libc-install-debug
export CFLAGS="-g -Og"
mkdir -p avr-libc/obj-avr-debug
cd avr-libc/obj-avr-debug
../configure --prefix=$PREFIX --build=`../config.guess` --host=avr >& logConfigure
nice make -j8 >& logMake
make install >& logInstall
cd ../..

### opt
export PREFIX=`pwd`/avr-libc-install-opt
export CFLAGS="-Os -flto -Wstrict-aliasing -fstrict-aliasing"
mkdir -p avr-libc/obj-avr-opt
cd avr-libc/obj-avr-opt
../configure --prefix=$PREFIX --build=`../config.guess` --host=avr >& logConfigure
nice make -j8 >& logMake
make install >& logInstall
cd ../..
