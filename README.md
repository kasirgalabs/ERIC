# ERIC - Encryption tool for RISCV with Compiler #
ERIC is an LLVM-based compiler that provides many encryption options for riscv-based systems.

By using ERIC, you can make the programs you compile run on authorized hardware. For this, you need to have a key or password synthesized in your user processor.

In addition, it can be used in cases where you want the data kept in memory to remain encrypted or depending on the needs such as keeping the program securely.

ERIC offers fully customizable encryption. There are currently 3 different encryption methods supported:

### [1. Instruction-Level Full Encryption](https://github.com/kasirgalabs/ERIC/blob/main/README.md#1-instruction-level-full-encryption-1) ###
   
   This method encrypts the instructions one by one. It provides encryption of all instructions in the program by entering a specific key.
   
### [2. Instruction-Level Partial Encryption](https://github.com/kasirgalabs/ERIC/blob/main/README.md#2-instruction-level-partial-encryption-1) ###
 
   This method supports unlimited customization. You can choose the types of instructions you think are critical to your program and only encrypt them. You can also encrypt each instruction to correspond to different bits within itself.
 
### [3. Memory-Level Encryption](https://github.com/kasirgalabs/ERIC/blob/main/README.md#3-memory-level-encryption-1) ###
 
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

#### Recommended Light Installation ####
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

# Building Repository #

### 1. Building kasirga Compiler and alp Encryption Tool ###

**1-)** Go to **kasirga-compiler-and-alp** directory. Then create a build directory and change directory:
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

You can look for changing compiler that I answered on [stackoverflow](https://stackoverflow.com/questions/68349442/how-to-fix-undefined-reference-llvm-error-while-linking-cxx-executable/68568867#68568867).

**Now you can find your executables in /kasirga-compiler-and-alp/build/bin folder as alp and kasirga variants.**

### 2. Building kasirgaIDE ###
**1-)** Go to **kasirgaIDE** directory. Then create a build directory and change directory:
```bash
cd kasirgaIDE
mkdir build
cd build
```
**2-)** Configure with cmake:
```bash
cmake -G Ninja ..
```

**3-)** Build with cmake or make:

```bash
cmake --build .
```

Alternative build with make:

```bash
make
```

# Usage of Tools and Compiler #

## kasirga ##

You can use clang-like compiler but if you compile a .c code as object code it will also run alp obfuscator and give encrypted or non-encrypted hex code.

**Example usages:**

Host pc executable:

```bash
kasirga example.c -o example
```

Host pc assembly code:

```bash
/home/shc/Desktop/kasirga-compiler/build/bin/kasirga -S example.c -o example.s 
```

Host pc llvm ir code:

```bash
/home/shc/Desktop/kasirga-compiler/build/bin/kasirga -S -emit-llvm example.c -o example.ll 
```

Host pc object code:

```bash
/home/shc/Desktop/kasirga-compiler/build/bin/kasirga -c example.c -o example.o
```

Riscv32 object code (also runs alp):

```bash
/home/shc/Desktop/kasirga-compiler/build/bin/kasirga -c -target riscv32-unknown-elf --sysroot=/home/shc/riscv-new/_install/riscv64-unknown-elf --gcc-toolchain=/home/shc/riscv-new/_install/ example.c -o example.o
```

I am using --sysroot and --gcc-toolchain flags to compile for riscv. You need to have riscv-gnu-toolchain pre installed.
For --sysroot and --gcc-toolchain flags you can look here that I answered: https://stackoverflow.com/questions/68580399/using-clang-to-compile-for-risc-v

## alp ##

You can use to obfuscate any compiled object code to non-encrypted or encrypted hex code. For encryption you have three options:

### [1. Instruction-Level Full Encryption](https://github.com/kasirgalabs/ERIC/blob/main/README.md#1-instruction-level-full-encryption) ###

We can use `--enckeyall` flag to encrypt all rv32i, rv32m, rv32a, rv32f, rv32d, rv64i, rv64m, rv64a, rv64f, rv64d, rvc quadrant 0, rvc quadrant 1 and rvc quadrant 2 instructions. (Not supported RSA encryption for now.)

**Usage of the flag:** 
```bash
--enckeyall="<your32bitkeyasbinary>(for 16 bit compressed instructions it uses most significant(left) 16 bit of this as key)"`
```

**Example usage:** 
```bash
/home/shc/ERIC/kasirga-compiler-and-alp/build/bin/alp --enckeyall="10100100101000000000001000000101" -d example.o`
```

This will xor all instructions (all of hex code) with given enckeyall.

(in example

**32 bit instructions (rv32, rv64)** xor **10100100101000000000001000000101**

**16 bit instructions (rvc)** xor **1010010010100000**
)


### [2. Instruction-Level Partial Encryption](https://github.com/kasirgalabs/ERIC/blob/main/README.md#2-instruction-level-partial-encryption) ###

We have two options to encrypt hex code partially, extension specific and instruction specific. 

#### 2.1. Instruction-Level Partial Extension Specific Encryption ####

We have 15 flags to extension specific instruction encryption:

```bash
--ienc32insts="<your47bitrv32iinstlistasbinary(use 1 to encrypt corresponding instruction)>"
--menc32insts="<your8bitrv32minstlistasbinary(use 1 to encrypt corresponding instruction)>"
--aenc32insts="<your11bitrv32ainstlistasbinary(use 1 to encrypt corresponding instruction)>"
--fenc32insts="<your26bitrv32finstlistasbinary(use 1 to encrypt corresponding instruction)>"
--denc32insts="<your26bitrv32dinstlistasbinary(use 1 to encrypt corresponding instruction)>"

--ienc64insts="<your15bitrv64iinstlistasbinary(use 1 to encrypt corresponding instruction)>"
--menc64insts="<your5bitrv64minstlistasbinary(use 1 to encrypt corresponding instruction)>"
--aenc64insts="<your11bitrv64ainstlistasbinary(use 1 to encrypt corresponding instruction)>"
--fenc64insts="<your4bitrv64finstlistasbinary(use 1 to encrypt corresponding instruction)>"
--denc64insts="<your6bitrv64dinstlistasbinary(use 1 to encrypt corresponding instruction)>"

--cencq0insts="<your11bitrvcq0instlistasbinary(use 1 to encrypt corresponding instruction)>"
--cencq1insts="<your21bitrvcq1instlistasbinary(use 1 to encrypt corresponding instruction)>"
--cencq2insts="<your17bitrvcq2instlistasbinary(use 1 to encrypt corresponding instruction)>"
```

and 15 key flags (+2 unsupported rvq extension flags) to encrypt given instructions by extension:

```bash
--ienc32key="<your32bitkeyasbinary>"
--menc32key="<your32bitkeyasbinary>"
--aenc32key="<your32bitkeyasbinary>"
--fenc32key="<your32bitkeyasbinary>"
--denc32key="<your32bitkeyasbinary>"

*** --qenc32key="<your32bitkeyasbinary>" ***  --> not supported yet

--ienc64key="<your32bitkeyasbinary>"
--menc64key="<your32bitkeyasbinary>"
--aenc64key="<your32bitkeyasbinary>"
--fenc64key="<your32bitkeyasbinary>"
--denc64key="<your32bitkeyasbinary>"

*** --qenc64key="<your32bitkeyasbinary>" ***  --> not supported yet

--cencq0key="<your16bitkeyasbinary>"
--cencq1key="<your16bitkeyasbinary>"
--cencq2key="<your16bitkeyasbinary>"
```

Also we have instruction flags as seperately instead of instruction bit lists (they can be used interchangeably):

<details> <summary> <b> Encryptable rv32i Instruction List (47 instructions, most left bit is 0. most right bit is 46. instruction of --ienc32insts) </b> </summary>
   
***0 -)*** ``` --lui ```

***1 -)*** ``` --auipc ```

***2 -)*** ``` --jal ```

***3 -)*** ``` --jalr ```

***4 -)*** ``` --beq ```

***5 -)*** ``` --bne ```

***6 -)*** ``` --blt ```

***7 -)*** ``` --bge ```

***8 -)*** ``` --bltu ```

***9 -)*** ``` --bgeu ```

***10-)*** ``` --lb ```

***11-)*** ``` --lh ```

***12-)*** ``` --lw ```

***13-)*** ``` --lbu ```

***14-)*** ``` --lhu ```

***15-)*** ``` --sb ```

***16-)*** ``` --sh ```

***17-)*** ``` --sw ```

***18-)*** ``` --addi ```

***19-)*** ``` --slti ```

***20-)*** ``` --sltiu ```

***21-)*** ``` --xori ```

***22-)*** ``` --ori ```

***23-)*** ``` --andi ```

***24-)*** ``` --slli ```

***25-)*** ``` --srli ```

***26-)*** ``` --srai ```

***27-)*** ``` --add ```

***28-)*** ``` --sub ```

***29-)*** ``` --sll ```

***30-)*** ``` --slt ```

***31-)*** ``` --sltu ```

***32-)*** ``` --xor_ ```

***33-)*** ``` --srl ```

***34-)*** ``` --sra ```

***35-)*** ``` --or_ ```

***36-)*** ``` --and_ ```

***37-)*** ``` --fence ```

***38-)*** ``` --fence_i ```

***39-)*** ``` --ecall ```

***40-)*** ``` --ebreak ```

***41-)*** ``` --csrrw ```

***42-)*** ``` --csrrs ```

***43-)*** ``` --csrrc ```

***44-)*** ``` --csrrwi ```

***45-)*** ``` --csrrsi ```

***46-)*** ``` --csrrci ```
   
</details>


<details> <summary> <b> Encryptable rv32m Instruction List (8 instructions, most left bit is 0. most right bit is 7. instruction of --menc32insts) </b> </summary>
   
***0 -)*** ``` --mul ```

***1 -)*** ``` --mulh ``` 

***2 -)*** ``` --mulhsu ``` 

***3 -)*** ``` --mulhu ``` 

***4 -)*** ``` --div_ ```

***5 -)*** ``` --divu ``` 

***6 -)*** ``` --rem ```

***7 -)*** ``` --remu ```

</details>

<details> <summary> <b> Encryptable rv32a Instruction List (11 instructions, most left bit is 0. most right bit is 10. instruction of --aenc32insts) </b> </summary>
   
***0 -)*** ``` --lr_w ```

***1 -)*** ``` --sc_w ```

***2 -)*** ``` --amoswap_w ```

***3 -)*** ``` --amoadd_w ```

***4 -)*** ``` --amoxor_w ```

***5 -)*** ``` --amoand_w ```

***6 -)*** ``` --amoor_w ```

***7 -)*** ``` --amomin_w ```

***8 -)*** ``` --amomax_w ```

***9 -)*** ``` --amominu_w ```

***10-)*** ``` --amomaxu_w ```

</details>

<details> <summary> <b> Encryptable rv32f Instruction List (26 instructions, most left bit is 0. most right bit is 25. instruction of --fenc32insts) </b> </summary>
   
***0 -)*** ``` --flw ```

***1 -)*** ``` --fsw ```

***2 -)*** ``` --fmadd_s ```

***3 -)*** ``` --fmsub_s ```

***4 -)*** ``` --fnmsub_s ```

***5 -)*** ``` --fnmadd_s ```

***6 -)*** ``` --fadd_s ```

***7 -)*** ``` --fsub_s ```

***8 -)*** ``` --fmul_s ```

***9 -)*** ``` --fdiv_s ```

***10-)*** ``` --fsqrt_s ```

***11-)*** ``` --fsgnj_s ```

***12-)*** ``` --fsgnjn_s ```

***13-)*** ``` --fsgnjx_s ```

***14-)*** ``` --fmin_s ```

***15-)*** ``` --fmax_s ```

***16-)*** ``` --fcvt_w_s ```

***17-)*** ``` --fcvt_wu_s ```

***18-)*** ``` --fmv_x_w ```

***19-)*** ``` --feq_s ```

***20-)*** ``` --flt_s ```

***21-)*** ``` --fle_s ```

***22-)*** ``` --fclass_s ```

***23-)*** ``` --fcvt_s_w ```

***24-)*** ``` --fcvt_s_wu ```

***25-)*** ``` --fmv_w_x ```

</details>

<details> <summary> <b> Encryptable rv32d Instruction List (26 instructions, most left bit is 0. most right bit is 25. instruction of --denc32insts) </b> </summary>
   
***0 -)*** ``` --fld ```

***1 -)*** ``` --fsd ```

***2 -)*** ``` --fmadd_d ```

***3 -)*** ``` --fmsub_d ```

***4 -)*** ``` --fnmsub_d ```

***5 -)*** ``` --fnmadd_d ```

***6 -)*** ``` --fadd_d ```

***7 -)*** ``` --fsub_d ```

***8 -)*** ``` --fmul_d ```

***9 -)*** ``` --fdiv_d ```

***10-)*** ``` --fsqrt_d ```

***11-)*** ``` --fsgnj_d ```

***12-)*** ``` --fsgnjn_d ```

***13-)*** ``` --fsgnjx_d ```

***14-)*** ``` --fmin_d ```

***15-)*** ``` --fmax_d ```

***16-)*** ``` --fcvt_s_d ```

***17-)*** ``` --fcvt_d_s ```

***18-)*** ``` --feq_d ```

***19-)*** ``` --flt_d ```

***20-)*** ``` --fle_d ```

***21-)*** ``` --fclass_d ```

***22-)*** ``` --fcvt_w_d ```

***23-)*** ``` --fcvt_wu_d ```

***24-)*** ``` --fcvt_d_w ```

***25-)*** ``` --fcvt_d_wu ```

</details>

<details> <summary> <b> <i> (Not supported yet) Encryptable rv32q Instruction List (28 instructions, most left bit is 0. most right bit is 27. instruction of --qenc32insts) </i> </b> </summary>

```bash

***0 -)*** ``` --flq ```  --> *** not supported yet ***

***1 -)*** ``` --fsq ```  --> *** not supported yet ***

***2 -)*** ``` --fmadd_q ```  --> *** not supported yet ***

***3 -)*** ``` --fmsub_q ```  --> *** not supported yet ***

***4 -)*** ``` --fnmsub_q ```  --> *** not supported yet ***

***5 -)*** ``` --fnmadd_q ```  --> *** not supported yet ***

***6 -)*** ``` --fadd_q ```  --> *** not supported yet ***

***7 -)*** ``` --fsub_q ```  --> *** not supported yet ***

***8 -)*** ``` --fmul_q ```  --> *** not supported yet ***

***9 -)*** ``` --fdiv_q ```  --> *** not supported yet ***

***10-)*** ``` --fsqrt_q ```  --> *** not supported yet ***

***11-)*** ``` --fsgnj_q ```  --> *** not supported yet ***

***12-)*** ``` --fsgnjn_q ```  --> *** not supported yet ***

***13-)*** ``` --fsgnjx_q ```  --> *** not supported yet ***

***14-)*** ``` --fmin_q ```  --> *** not supported yet ***

***15-)*** ``` --fmax_q ```  --> *** not supported yet ***

***16-)*** ``` --fcvt_s_q ```  --> *** not supported yet ***

***17-)*** ``` --fcvt_q_s ```  --> *** not supported yet ***

***18-)*** ``` --fcvt_d_q ```  --> *** not supported yet ***

***19-)*** ``` --fcvt_q_d ```  --> *** not supported yet ***

***20-)*** ``` --feq_q ```  --> *** not supported yet ***

***21-)*** ``` --flt_q ```  --> *** not supported yet ***

***22-)*** ``` --fle_q ```  --> *** not supported yet ***

***23-)*** ``` --fclass_q ```  --> *** not supported yet ***

***24-)*** ``` --fcvt_w_q ```  --> *** not supported yet ***

***25-)*** ``` --fcvt_wu_q ```  --> *** not supported yet ***

***26-)*** ``` --fcvt_q_w ```  --> *** not supported yet ***

***27-)*** ``` --fcvt_q_wu ```  --> *** not supported yet ***

```

</details>

<details> <summary> <b> Encryptable rv64i Instruction List (15 instructions, most left bit is 0. most right bit is 14. instruction of --ienc64insts) </b> </summary>

***0 -)*** ``` --lwu ```

***1 -)*** ``` --ld ```

***2 -)*** ``` --sd ```

```bash

*** 3 -) *** --slli  --> *** not supported yet ***

*** 4 -) *** --srli  --> *** not supported yet ***

*** 5 -) *** --srai  --> *** not supported yet ***

```

***6 -)*** ``` --addiw ```

***7 -)*** ``` --slliw ```

***8 -)*** ``` --srliw ```

***9 -)*** ``` --sraiw ```

***10-)*** ``` --addw ```

***11-)*** ``` --subw ```

***12-)*** ``` --sllw ```

***13-)*** ``` --srlw ```

***14-)*** ``` --sraw ```

</details>

<details> <summary> <b> Encryptable rv64m Instruction List (5 instructions, most left bit is 0. most right bit is 4. instruction of --menc64insts) </b> </summary>

***0 -)*** ``` --mulw ```

***1 -)*** ``` --divw ```

***2 -)*** ``` --divuw ```

***3 -)*** ``` --remw ```

***4 -)*** ``` --remuw ```

</details>

<details> <summary> <b> Encryptable rv64a Instruction List (11 instructions, most left bit is 0. most right bit is 10. instruction of --aenc64insts) </b> </summary>

***0 -)*** ``` --lr_d ```

***1 -)*** ``` --sc_d ```

***2 -)*** ``` --amoswap_d ```

***3 -)*** ``` --amoadd_d ```

***4 -)*** ``` --amoxor_d ```

***5 -)*** ``` --amoand_d ```

***6 -)*** ``` --amoor_d ```

***7 -)*** ``` --amomin_d ```

***8 -)*** ``` --amomax_d ```

***9 -)*** ``` --amominu_d ```

***10-)*** ``` --amomaxu_d ```

</details>

<details> <summary> <b> Encryptable rv64f Instruction List (4 instructions, most left bit is 0. most right bit is 3. instruction of --fenc64insts) </b> </summary>

***0 -)*** ``` --fcvt_l_s ```

***1 -)*** ``` --fcvt_lu_s ```

***2 -)*** ``` --fcvt_s_l ```

***3 -)*** ``` --fcvt_s_lu ```

</details>

<details> <summary> <b> Encryptable rv64d Instruction List (6 instructions, most left bit is 0. most right bit is 5. instruction of --fenc64insts) </b> </summary>

***0 -)*** ``` --fcvt_l_d ```

***1 -)*** ``` --fcvt_lu_d ```

***2 -)*** ``` --fmv_x_d ```

***3 -)*** ``` --fcvt_d_l ```

***4 -)*** ``` --fcvt_d_lu ```

***5 -)*** ``` --fmv_d_x ```

</details>

<details> <summary> <b> <i> (Not supported yet) Encryptable rv64q Instruction List (4 instructions, most left bit is 0. most right bit is 3. instruction of --qenc64insts) </i> </b> </summary>

```bash

***0 -)*** --fcvt_l_q  --> *** not supported yet ***

***1 -)*** --fcvt_lu_q  --> *** not supported yet ***

***2 -)*** --fcvt_q_l  --> *** not supported yet ***

***3 -)*** --fcvt_q_lu  --> *** not supported yet ***

```

</details>

<details> <summary> <b> Encryptable rvc quadrant 0 Instruction List (11 instructions, most left bit is 0. most right bit is 10. instruction of --cencq0insts) </b> </summary>

***0 -)*** ``` --c_addi4spn ```

***1 -)*** ``` --c_fld ```

```bash
***2 -)*** ``` --c_lq ```  --> *** not supported yet ***
```

***3 -)*** ``` --c_lw ```

***4 -)*** ``` --c_flw ```

***5 -)*** ``` --c_ld ```

***6 -)*** ``` --c_fsd ```

```bash
***7 -)*** ``` --c_sq ```  --> *** not supported yet ***
```

***8 -)*** ``` --c_sw ```

***9 -)*** ``` --c_fsw ```

***10-)*** ``` --c_sd ```

</details>

<details> <summary> <b> Encryptable rvc quadrant 1 Instruction List (21 instructions, most left bit is 0. most right bit is 20. instruction of --cencq1insts) </b> </summary>

***0 -)*** ``` --c_nop ```

***1 -)*** ``` --c_addi ```

***2 -)*** ``` --c_jal ```

***3 -)*** ``` --c_addiw ```

***4 -)*** ``` --c_li ```

***5 -)*** ``` --c_addi16sp ```

***6 -)*** ``` --c_lui ```

***7 -)*** ``` --c_srli ```

```bash
***8 -)*** ``` --c_srli64 ```  --> *** not supported yet ***
```

***9 -)*** ``` --c_srai ```

```bash
***10-)*** ``` --c_srai64 ```  --> *** not supported yet ***
```

***11-)*** ``` --c_andi ```

***12-)*** ``` --c_sub ```

***13-)*** ``` --c_xor ```

***14-)*** ``` --c_or ```

***15-)*** ``` --c_and ```

***16-)*** ``` --c_subw ```

***17-)*** ``` --c_addw ```

***18-)*** ``` --c_j ```

***19-)*** ``` --c_beqz ```

***20-)*** ``` --c_bnez ```

</details>

<details> <summary> <b> Encryptable rvc quadrant 2 Instruction List (17 instructions, most left bit is 0. most right bit is 16. instruction of --cencq2insts) </b> </summary>

***0 -)*** ``` --c_slli ```

```bash
***1 -)*** ``` --c_slli64 ```  --> *** not supported yet ***
```

***2 -)*** ``` --c_fldsp ```

```bash
***3 -)*** ``` --c_lqsp ```  --> *** not supported yet ***
```

***4 -)*** ``` --c_lwsp ```

***5 -)*** ``` --c_flwsp ```

***6 -)*** ``` --c_ldsp ```

***7 -)*** ``` --c_jr ```

***8 -)*** ``` --c_mv ```

***9 -)*** ``` --c_ebreak ```

***10-)*** ``` --c_jalr ```

***11-)*** ``` --c_add ```

***12-)*** ``` --c_fsdsp ```

```bash
***13-)*** ``` --c_sqsp  --> *** not supported yet ***
```

***14-)*** ``` --c_swsp ```

***15-)*** ``` --c_fswsp ```

***16-)*** ``` --c_sdsp ```

</details>

#### 2.2. Instruction-Level Partial Instruction Specific Encryption ####

For any instruction, we can encrypt with 

### [3. Memory-Level Encryption](https://github.com/kasirgalabs/ERIC/blob/main/README.md#3-memory-level-encryption) ###







# Screenshots #
![alt text](screenshots/ide1editor.png)

![alt text](screenshots/ide2menubar.png)

![alt text](screenshots/ide3menubar.png)

![alt text](screenshots/ide4enc.png)

![alt text](screenshots/ide5enc.png)

![alt text](screenshots/ide6parenc.png)

![alt text](screenshots/ide7parenc.png)

