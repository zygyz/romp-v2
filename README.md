


This readme is mainly for the purpose of showing how to build romp directly with cmake.
For a release version of romp, we encourage installation with spack. 

### System Requirements
1. Operating Systems:  Linux

2. Architecture:  x86_64

3. Compiler: gcc 9.2.0 (recommended); any compiler supporting c++17

### Prerequisites
Checkout my version of spack, which contains some modification to package.py 
for llvm-openmp 
and also the pacakge spec for romp.

Please use the branch `romp-build`

`git clone git@github.com:zygyz/spack.git`

### Build ROMP without using spack
Goal: Build ROMP directly using cmake so that changes to ROMP and dyninst
could be built and tested without pushing changes to git repo. 

However, we still use spack to install some dependent libraries: 
```
gflags glog llvm-openmp
```
Remember to install with c++17 compatible compiler option. e.g.,
```
spack install gflags %gcc@9.2.0
```
Configure the compiler option by following the steps in 
https://spack-tutorial.readthedocs.io/en/latest/tutorial_configuration.html
```
spack config edit compilers 
```
1. Build dyninst. Suppose the dyninst is located in `/path/to/dyninst`, and 
 the artifact is installed in `path/to/dyninst/install`. Create a symlink:
 ``` ln -s /path/to/dyninst/install $HOME/dyninst```
 
2. set environement variables
```
export GLOG_PREFIX=`spack location --install-dir glog`
export GFLAGS_PREFIX=`spack location --install-dir gflags`
export LLVM_PREFIX=`spack location --install-dir llvm-openmp`
export CUSTOM_DYNINST_PREFIX=$HOME/dyninst
export LIBRARY_PATH=`spack location --install-dir glog`/lib\
`spack location --install-dir llvm-openmp`/lib
```
3. Change directory to `romp-v2`: 
```mkdir build
   mkdir install
   cd build
   cmake -DCMAKE_PREFIX_PATH="$GFLAGS_PREFIX;$GLOG_PREFIX;$CUSTOM_DYNINST_PREFIX"
         -DLLVM_PATH=$LLVM_PREFIX -DCMAKE_CXX_FLAGS=-std=c++11 -DCUSTOM_DYNINST=ON 
         -DCMAKE_INSTALL_PREFIX=`pwd`/../install ..
   make
   make install
 ```
4. Now dyninst client `InstrumentMain` is installed in `romp-v2/install/bin`
   Before running instrumentation, set up several environment variables:
  ```
   export ROMP_PATH=/path/to/romp-v2/install/lib/libomptrace.so
   export DYNINSTAPI_RT_LIB=$HOME/dyninst/lib/libdyninstAPI_RT.so
   export LD_LIBRARY_PATH=`spack location --install-dir glog`/lib:\
                          `spack location --install-dir llvm-openmp`/lib:\
                           $HOME/dyninst/lib
  ```
5. Now compile `tests/test_lib_inst.cpp` with:
```
g++ test.cpp -std=c++11 -lomp- fopenmp
```
6. Then, go to `romp-v2/install/bin`, 
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
