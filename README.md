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

# Installation For Linux #

### 1. LLVM Library Installation ###

#### 1.1 CMake and Ninja Installation ####
```bash
sudo snap install cmake --classic

sudo apt install ninja-build
```

#### 1.2 LLVM Installation ####
```bash
git clone https://github.com/llvm/llvm-project.git && cd llvm-project && mkdir build && cd build && cmake -G Ninja -DLLVM_ENABLE_PROJECTS=clang -DLLVM_TARGETS_TO_BUILD=all -DLLVM_ENABLE_LIBCXX=ON -DCMAKE_BUILD_TYPE=Release -DLLVM_INSTALL_UTILS=ON -DBUILD_SHARED_LIBS=True -DLLVM_USE_SPLIT_DWARF=True -DLLVM_OPTIMIZED_TABLEGEN=True -DLLVM_BUILD_TESTS=True -DLLVM_PARALLEL_LINK_JOBS=1 ../llvm && cmake --build .
```

### 2. wxWidgets Library Installation ###

