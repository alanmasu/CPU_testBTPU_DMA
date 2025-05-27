#include <main.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <BinaryMatMul.h>
#include <UART.h>



int main(int argc, char const *argv[]){
    for(int i = 0; i < BINARY_FRAG_SIZE; ++i){
        *(BTPU0_W_MEMORY)[i] = i;
        *(BTPU0_IO0_MEMORY)[i] = i + 1;
        *(BTPU0_IO1_MEMORY)[i] = i + 2;
    }

    //Configure first Binary Matrix Mul 
    btpuSetAddrs(BTPU0RegFile, 0, 0, 1);
    btpuStartBinaryMatrixMul(BTPU0RegFile, 30, false, BTPU_OUT_MEMORY_0_CONFIG);

    btpuWaitBinaryMatrixMul(BTPU0RegFile);

    //Configure second Binary Matrix Mul
    btpuSetAddrs(BTPU0RegFile, 0, 0, 1);
    btpuStartBinaryMatrixMul(BTPU0RegFile, 30, false, BTPU_OUT_MEMORY_1_CONFIG);

    btpuWaitBinaryMatrixMul(BTPU0RegFile);

    // Trap CPU
    while(1);
}
