## Please refer to the readme in experimental branch for now




### System Requirements
1. Operating Systems:  Linux

2. Architecture:  x86_64

### Prerequisites
Checkout my version of spack, which contains some modification to package.py 
for llvm-openmp 
and also the pacakge spec for romp.

`git clone git@github.com:zygyz/spack.git`


### Installing
`spack install romp@develop`

#### Debug library instrumentation problem
First, set environment variables
```
export ROMP_PATH=`spack location --install-dir romp`/lib/libomptrace.so

export LD_LIRBARY_PATH=`spack location --install-dir dyninst`/lib:\
`spack location --install-dir llvm-openmp`/lib:\
`spack location --install-dir romp`/lib

export LIBRARY_PATH=`spack location --install-dir llvm-openmp`/lib:\
`spack location --install-dir romp`/lib

export CPLUS_INCLUDE_PATH=`spack location --install-dir llvm-openmp`/include
``` 
Then, compile `tests/test_lib_inst.cpp` with:
```
g++ test.cpp -std=c++11 -lomp- fopenmp
```
Then, get the instrument client located in
`spack location --install-dir romp`/bin/InstrumentMain`

```
./InstrumentMain --program=./a.out
```
This will generate a.out.inst

```
LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH ./a.out.inst
```

The dyninst client code is in `InstrumentClient`. Core functions are in 
`InstrumentClient.cpp`. Library names are listed in `skipLibraryName` 
vector. Currently, three libraries could be instrumented and linked without
generating segmentation fault: `libomp.so, libgromp.so.1, libm.so.6`. 

### Build local version of dyninst

Sometimes, it would be convenient to build dyninst on local machine with 
appropriate modification for the purpose of debugging. To enable this,
one can put dyninst source code in the home directory: `$HOME/dyninst`, then
one can install romp with the following command:

```
spack install romp@develop +debug_dyninst 
```
