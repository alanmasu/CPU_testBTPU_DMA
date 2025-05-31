#include <main.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <BinaryMatMul.h>
#include <UART.h>

// #define SIMULATION

#ifndef SIMULATION
    #define PRINTF_DBG(...) printf(__VA_ARGS__)
#else 
    #define PRINTF_DBG(...)
#endif



int main(int argc, char const *argv[]){
    const int B_M = 2;
    const int B_N = 3;
    const int B_K = 4;
    const uint32_t signCmp = 48;

    const int M = B_M * BINARY_FRAG_SIZE;
    const int N = B_N * BINARY_FRAG_SIZE;
    const int K = B_K * BINARY_FRAG_SIZE;
 
    BinaryMatrix_t A = (BinaryMatrix_t)malloc(M * (N / 32) * sizeof(uint32_t)); //768 byte
    // BinaryMatrix_t A2 = (BinaryMatrix_t)malloc(M * (N / 32) * sizeof(uint32_t)); //768 byte

    BinaryMatrix_t W = (BinaryMatrix_t)malloc(N * (K / 32) * sizeof(uint32_t)); // 1.536 kB

    BinaryMatrix_t O = (BinaryMatrix_t)malloc(M * (K / 32) * sizeof(uint32_t)); // 1 kB
    // BinaryMatrix_t O2 = (BinaryMatrix_t)malloc(M * (K / 32) * sizeof(uint32_t)); // 1 kB

    BinaryMatrix_t OSerial = (BinaryMatrix_t)malloc(M * (K / 32) * sizeof(uint32_t)); // 1 kB
    // BinaryMatrix_t O2Serial = (BinaryMatrix_t)malloc(M * (K / 32) * sizeof(uint32_t)); // 1 kB

    if(!A || !W || !O || !OSerial){
        PRINTF_DBG("Memory allocation failed!\n");
        while(1);
    }

    PRINTF_DBG("Memory allocation successful!\n");

    PRINTF_DBG("\nInitializing matrices...\n");
    for(int i = 0; i < M * (N / 32); ++i){
        A[i] = i + 1;
    }
    for(int i = 0; i < N * (K / 32); ++i){
        W[i] = i;
    }
    memset(O, 0, M * (K / 32) * sizeof(uint32_t));

    PRINTF_DBG("\nLoading matices to BTPU fragments...\n");
    loadBinaryMatrixToFragments(A, BTPU0_IO0_MEMORY, M, N);
    loadBinaryMatrixToFragments(W, BTPU0_W_MEMORY, N, K);

    btpuSetBlocks(BTPU0RegFile, B_M, B_N, B_K);
    btpuSetAddrs(BTPU0RegFile, 0, 0, B_N * B_K); 

    PRINTF_DBG("\nStarting first multiplication...\n");
    btpuStartBinaryMatrixMul(BTPU0RegFile, signCmp, true, true, BTPU_USE_MEMORY_0_CONFIG);
    btpuWaitBinaryMatrixMul(BTPU0RegFile);
    storeFramentsToBinaryMatrix(BTPU0_IO0_MEMORY, O, M, K);

    fastBinaryMatrixMul(A, W, OSerial, M, N, K, signCmp);

    bool success = true;
    int err = 0;
    for(int i = 0; i < M * (K / 32); ++i){
        if(O[i] != OSerial[i]){
            if(success) PRINTF_DBG("Mismatch in O at index %d: BTPU = 0x%08x, Serial = 0x%08x\n", i, O[i], OSerial[i]);
            success = false;
            ++err;
        }
    }
    if(success){
        PRINTF_DBG("Test #1: OK\n");
    }else{
        PRINTF_DBG("   %d more errors found\n", err);
        PRINTF_DBG("Test #1: FAILED\n");
    }

    for(int i = 0; i < M * (N / 32); ++i){
        A[i] = i + 1;
    }

    PRINTF_DBG("\nStarting second multiplication...\n");
    btpuSetBlocks(BTPU0RegFile, B_M, B_N, B_K);
    btpuSetAddrs(BTPU0RegFile, 0, 0, B_N * B_K);
    btpuStartBinaryMatrixMul(BTPU0RegFile, signCmp, true, true, BTPU_USE_MEMORY_0_CONFIG);
    btpuWaitBinaryMatrixMul(BTPU0RegFile);
    storeFramentsToBinaryMatrix(BTPU0_IO1_MEMORY, O, M, K);
    fastBinaryMatrixMul(A, W, OSerial, M, N, K, signCmp);

    err = 0;
    success = true;
    for(int i = 0; i < M * (K / 32); ++i){
        if(O[i] != OSerial[i]){
            if(success) PRINTF_DBG("Mismatch in O at index %d: BTPU = 0x%08x, Serial = 0x%08x\n", i, O[i], OSerial[i]);
            success = false;
            ++err;
        }
    }
    if(success){
        PRINTF_DBG("Test #2: OK\n");
    }else{
        PRINTF_DBG("   %d more errors found\n", err);
        PRINTF_DBG("Test #2: FAILED\n");
    }

    PRINTF_DBG("All tests completed!\n");
    PRINTF_DBG("Freeing allocated memory...\n");
    free(A);
    // free(A2);
    free(W);
    free(O);
    // free(O2);
    free(OSerial);
    // free(O2Serial);
    
    PRINTF_DBG("Setting BRAM Port to EXTERNAL...\n");
    BTPU0RegFile->creg.reg.BRAM_PORT_SEL = BTPU_BRAM_PORT_SEL_EXT; // Set BRAM port to external

    PRINTF_DBG("Trap CPU...\n");
    while(1);

    return 0;
}
