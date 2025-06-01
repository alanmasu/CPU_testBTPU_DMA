#include <main.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <BinaryMatMul.h>
#include <UART.h>
#include <GPIO.h>
#include <utilities.h>
#include <OLED.h>

// __attribute__((optimize("O0")))
void initMatrices(){
    for(int i = 0; i < BINARY_FRAG_SIZE; ++i){
        BTPU0_W_MEMORY[0][i] = i;
        BTPU0_IO0_MEMORY[0][i] = i + 1;
        BTPU0_IO1_MEMORY[0][i] = i + 2;
    }
}

int main(int argc, char const *argv[]){
    // Initialize Matrices
    initMatrices();

    //Configure first Binary Matrix Mul 
    btpuSetAddrs(BTPU0RegFile, 0, 0, 1);
    btpuStartBinaryMatrixMul(BTPU0RegFile, 30, false, true, BTPU_USE_MEMORY_0_CONFIG);
    btpuWaitBinaryMatrixMul(BTPU0RegFile);

    // //Configure second Binary Matrix Mul
    btpuSetAddrs(BTPU0RegFile, 0, 0, 1);
    btpuStartBinaryMatrixMul(BTPU0RegFile, 30, false, true, BTPU_USE_MEMORY_1_CONFIG);

    btpuWaitBinaryMatrixMul(BTPU0RegFile);

    BTPU0RegFile->creg.reg.BRAM_PORT_SEL = BTPU_BRAM_PORT_SEL_EXT;
#ifndef SIMULATION
    printf("BTPU CREG:\n");
    printf("     start: %d\n", BTPU0RegFile->creg.reg.START);
    printf("     busy: %d\n", BTPU0RegFile->creg.reg.BUSY);
    printf("     omem_sel: %d\n", BTPU0RegFile->creg.reg.OMEM_SEL);
    printf("     bram_port_sel: %d\n", BTPU0RegFile->creg.reg.BRAM_PORT_SEL);
    printf("     acc_clear: %d\n", BTPU0RegFile->creg.reg.ACC_CLEAR);
    printf("     batched_mul: %d\n", BTPU0RegFile->creg.reg.BATCHED_MUL);
    printf("     error: %d\n", BTPU0RegFile->creg.reg.ERROR);
    printf("\n");
    printf("BTPU W Addr: %d\n", BTPU0RegFile->wMemStartAddr);
    printf("BTPU I Addr: %d\n", BTPU0RegFile->iMemStartAddr);
    printf("BTPU O Addr: %d\n", BTPU0RegFile->oMemStartAddr);
    printf("BTPU signCmp: %d\n", BTPU0RegFile->signCmp);
#endif

    // Trap CPU
    while(1);
}
