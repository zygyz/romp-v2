# ROMP

ROMP is a dynamic data race detector for OpenMP programs. 

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. 

### System Requirements
1. Operating Systems:  Linux

2. Architecture:  x86_64

### Prerequisites
Checkout my version of spack, which contains some modification to package.py for llvm-openmp 
and also the pacakge spec for romp.

`git clone git@github.com:zygyz/spack.git`


### Installing
`spack install romp@experimental`

### Caveats
    - For DRB047 in dataracebench, please use the byte level granularity checking otherwise the word level granularity checking causes false positives 
    - To switch from word level checking to byte level checking, disable the macro definition #define WORD_LEVEL and enable the macro definition #define BYTE_LEVEL and recompile romp library
    - For DRB114 in dataracebench, whether data race occurs is dependent on the control flow, i.e., the value of rand()%2 
    - DRB094,DRB095,DRB096,DRB112 require an OpenMP 4.5 compiler, currently romp does not test these programs
    - DRB116 tests use of target + teams constructs. The support for these two constructs is out of the scope of our SC18 paper. Adding support for target and teams constructs is subject to ongoing work.
## Authors

* **Yizi Gu (yg31@rice.edu)** - *Initial work* 

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
