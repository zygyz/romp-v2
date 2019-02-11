#!/bin/bash

root=`pwd`
#install spack
cd pkgs-src
git clone https://github.com/spack/spack.git
export PATH=$root/pkgs-src/spack/bin:$PATH

#build and install TCMalloc
spack install gperftools

##build and install dyninst
spack install dyninst

#build and install LLVM OpenMP runtime library 
cd $root
cd pkgs-src/llvm-openmp/openmp
mkdir llvm-openmp-build llvm-openmp-install 
cd llvm-openmp-build
cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++  -DCMAKE_INSTALL_PREFIX=`pwd`/../llvm-openmp-install ..    
make && make install
#
#
##build and install romp
cd $root
export CPATH=`pwd`/pkgs-src/llvm-openmp/openmp/llvm-openmp-install/include:$CPATH
export LD_LIBRARY_PATH=`pwd`/pkgs-src/llvm-openmp/openmp/llvm-openmp-install/lib:$LD_LIBRARY_PATH
cd pkgs-src/romp-lib
mkdir romp-build romp-install
cd romp-build
cmake -DCMAKE_INSTALL_PREFIX=`pwd`/../romp-install ..
make && make install

#build dyninst client
cd $root
cd pkgs-src/dyninst-client
make 
