# ERIC - Encryption tool for RISCV with Compiler #
ERIC is an LLVM-based compiler that provides many encryption options for riscv-based systems.

By using ERIC, you can make the programs you compile run on authorized hardware. For this, you need to have a key or password synthesized in your user processor.

In addition, it can be used in cases where you want the data kept in memory to remain encrypted or depending on the needs such as keeping the program securely.

ERIC offers fully customizable encryption. There are currently 3 different encryption methods supported:

### 1. Instruction - Level Full Encryption ###
   
   This method encrypts the instructions one by one. It provides encryption of all instructions in the program by entering a specific key.
   
### 2. Instruction - Level Partial Encryption ###
 
   This method supports unlimited customization. You can choose the types of instructions you think are critical to your program and only encrypt them. You can also encrypt each instruction to correspond to different bits within itself.
 
### 3. Memory-Level Encryption ###
 
   In this method, you can completely encrypt the program with a public key that we will use in ERIC. Unlike the previous options, all data expected to be in memory here is encrypted with the RSA encryption method. There is encryption not at the instruction level, but as much as the size of the program in memory. You can manually give the public and private key pairs to the system yourself, or you can ask ERIC to automatically generate a public key.


The thing to remember is that ERIC is a compiler with an interface developed for encryption. In order to run the encrypted programs you have compiled here, you need at least some hardware that does decryption. We will soon publish our hardware module that can work in integration with ERIC.


# Requirements #

* [LLVM library](https://github.com/llvm/llvm-project) built for RISCV target (LLVM installation)
* [wxWidgets library](https://github.com/wxWidgets/wxWidgets) built for IDE (wxWidgets installation)

# Required Installations For Linux #

## 1. LLVM Library Installation ##

### 1.1 Necessary Tools Installation ###

#### CMake Installation ####
```bash
sudo snap install cmake --classic
```

#### Ninja Installation ####
```bash
sudo apt install ninja-build
```

### 1.2 LLVM Installation ###

#### Recommended Installation ####
```bash
git clone https://github.com/llvm/llvm-project.git && \
cd llvm-project && \
mkdir build && \ cd build && \
cmake -G Ninja \
-DLLVM_ENABLE_PROJECTS=clang \ 
-DLLVM_TARGETS_TO_BUILD=all \
-DLLVM_ENABLE_LIBCXX=ON \
-DCMAKE_BUILD_TYPE=Release \
-DLLVM_INSTALL_UTILS=ON \
-DBUILD_SHARED_LIBS=True \
-DLLVM_USE_SPLIT_DWARF=True \
-DLLVM_OPTIMIZED_TABLEGEN=True \
-DLLVM_BUILD_TESTS=True \
-DLLVM_PARALLEL_LINK_JOBS=1 ../llvm && \
cmake --build .
```

#### Light Installation ####
```bash
git clone https://github.com/llvm/llvm-project.git && \
cd llvm-project && \
mkdir build && \
cd build && \
cmake -G Ninja \
-DLLVM_ENABLE_PROJECTS=clang \
-DLLVM_TARGETS_TO_BUILD=RISCV \
-DBUILD_SHARED_LIBS=True \
-DLLVM_PARALLEL_LINK_JOBS=1 ../llvm && \
cmake --build .
```

### 2. wxWidgets Library Installation ###

#### 2.1 Necessary Tools Installation ####
```bash
sudo apt install build-essential
```

```bash
sudo apt install libgtk-3-dev
```

#### 2.2 wxWidgets Installation ####
```bash
git clone https://github.com/wxWidgets/wxWidgets && \
mkdir gtk-build && \
cd gtk-build && \
../configure && \
make && \
sudo make install && \
sudo ldconfig
```

# Cloning Repository #
```bash
git clone https://github.com/kasirgalabs/ERIC
```
# Building kasirga Compiler and alp Encryption Tool #

**1-)** Go to kasirga-compiler-and-alp directory. Then create a build directory and change directory:
```bash
cd kasirga-compiler-and-alp
mkdir build
cd build
```

**2-)** Export your LLVM directories:

```bash
export LLVM_PROJECT_DIR={your-llvm-project-directory}
export LLVM_BUILD_DIR={your-llvm-install-or-build-directory}
```

Example:
```bash
export LLVM_PROJECT_DIR=~/llvm/llvm-project
export LLVM_BUILD_DIR=~/llvm/llvm-project/build
```

**3-)** Configure with cmake:

```bash
cmake -DLT_LLVM_INSTALL_DIR=$LLVM_BUILD_DIR -DCMAKE_MODULE_PATH=$LLVM_PROJECT_DIR/clang/cmake/modules ..
```

**4-)** Build with cmake or make:

```bash
cmake --build .
```

Alternative build with make:

```bash
make
```

If you can't build because of a compiler error, install a new compiler if does not exist, change your compiler as for example:

```bash
export CC=clang-11
export CXX=clang++-11
```

then delete build directory and start with the first step again.

You can look here for changing compiler that I answered: https://stackoverflow.com/questions/68349442/how-to-fix-undefined-reference-llvm-error-while-linking-cxx-executable/68568867#68568867

**Now you can find your executables in /kasirga-compiler/build/bin folder as alp and kasirga variants.**
