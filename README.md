# ERIC - Encrpytion for RISCV with Compiler 
ERIC is an LLVM-based compiler that provides many encryption options for riscv-based systems.

By using ERIC, you can make the programs you compile run on authorized hardware. For this, you need to have a key or password synthesized in your user processor.

In addition, it can be used in cases where you want the data kept in memory to remain encrypted or depending on the needs such as keeping the program securely.

ERIC offers fully customizable encryption. There are currently 3 different encryption methods supported:
1. Instruction - Level Full Encryption
   This method encrypts the instructions one by one. It provides encryption of all instructions in the program by entering a specific key.
2.Instruction - Level Partial Encryption
 This method supports unlimited customization. You can choose the types of instructions you think are critical to your program and only encrypt them. You can also encrypt each instruction to correspond to different bits within itself.
3. Memory-Level Encryption
 In this method, you can completely encrypt the program with a public key that we will use in ERIC. Unlike the previous options, all data expected to be in memory here is encrypted with the RSA encryption method. There is encryption not at the instruction level, but as much as the size of the program in memory. You can manually give the public and private key pairs to the system yourself, or you can ask ERIC to automatically generate a public key.
