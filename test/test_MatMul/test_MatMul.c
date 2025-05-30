#include <main.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <BinaryMatMul.h>
#include <UART.h>

#include "testHeader.h"

#ifndef SIMULATION
    #define PRINTF_DBG(...) printf(__VA_ARGS__)
#else 
    #define PRINTF_DBG(...)
#endif

int main(int argc, char const *argv[]){
    printf("Starting Binary Matrix Multiplication Test... (Test compiled at: %s %s)\n", __DATE__, __TIME__);
    int test_n = 0;
    const int B_M = 2;
    const int B_N = 3;
    const int B_K = 4;
    const uint32_t signCmp = 48;

    const int M = B_M * BINARY_FRAG_SIZE;
    const int N = B_N * BINARY_FRAG_SIZE;
    const int K = B_K * BINARY_FRAG_SIZE;

    BinaryMatrix_t A = (BinaryMatrix_t)malloc(M * (N / 32) * sizeof(uint32_t));
    BinaryMatrix_t W = (BinaryMatrix_t)malloc(N * (K / 32) * sizeof(uint32_t));
    BinaryMatrix_t O = (BinaryMatrix_t)malloc(M * (K / 32) * sizeof(uint32_t));

    Matrix_t result;

    result = (Matrix_t)argv;
    // result = (Matrix_t)malloc(M * K * sizeof(uint32_t));
    // result = (Matrix_t) 1;

    // if(!A || !W || !O || !result){
    // if(!A || !W || !O || !result){
    if(!A || !W || !result){
        PRINTF_DBG("Memory allocation failed!\n");
        printf("A = %p, W = %p, O = %p, result = %p\n", A, W, O, result);
        printf("argv = %p\n", argv);
        while(1);
    }else{
        PRINTF_DBG("Memory allocation successful!\n");
        printf("A = %p, W = %p, O = %p, result = %p\n", A, W, O, result);
    }


    PRINTF_DBG("\nInitializing matrices...\n");
    for(int i = 0; i < M * (N / 32); ++i){
        A[i] = i + 1;
    }
    for(int i = 0; i < N * (K / 32); ++i){
        W[i] = i;
    }

    // memset(O, 0, M * (K / 32) * sizeof(uint32_t));
    // memset(result, 0, M * K * sizeof(uint32_t));


    test_n = 1;
    binaryMatrixMul(A, W, (BinaryMatrix_t)result, M, N, K);
    binarizeMatrix(result, O, signCmp, M, K);

    bool success = true;
    int err = 0;
    for(int i = 0; i < M * (K / 32); ++i){
        if(O[i] != result_Matrix[i]){
            if(success) PRINTF_DBG("Test #1-%d: FAILED -> O[%d] was 0x%08x, Expected = 0x%08x\n", i, i, O[i], result_Matrix[i]);
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

    test_n = 2;
    fastBinaryMatrixMul(A, W, O, signCmp, M, N, K);
    success = true;
    err = 0;
    for(int i = 0; i < M * (K / 32); ++i){
        if(O[i] != result_Matrix[i]){
            if(success) PRINTF_DBG("Test #2-%d: FAILED -> O[%d] was 0x%08x, Expected = 0x%08x\n", i, i, O[i], result_Matrix[i]);
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

    test_n = 3;
    PRINTF_DBG("\nLoading BTPU fragments...\n");
    for(int i = 0; i < BINARY_FRAG_SIZE; ++i){
        BTPU0_W_MEMORY[0][i] = i;
        BTPU0_IO0_MEMORY[0][i] = i + 1;
        BTPU0_IO1_MEMORY[0][i] = i + 2;
    }
    PRINTF_DBG("Configuring BTPU...\n");
    // btpuSetAddrs(BTPU0RegFile, 0, 0, 1);
    // btpuStartBinaryMatrixMul(BTPU0RegFile, 30, false, true, BTPU_USE_MEMORY_0_CONFIG);

    

    // loadBinaryMatrixToFragments(A, BTPU0_IO0_MEMORY, M, N);
    // loadBinaryMatrixToFragments(W, BTPU0_W_MEMORY, N, K);
    // btpuSetBlocks(BTPU0RegFile, B_M, B_N, B_K);
    // btpuSetAddrs(BTPU0RegFile, 0, 0, B_N * B_K);
    // PRINTF_DBG("\nStarting first multiplication...\n");
    // btpuStartBinaryMatrixMul(BTPU0RegFile, signCmp, true, true, BTPU_USE_MEMORY_1_CONFIG);
    // PRINTF_DBG("Waiting for BTPU to finish...\n");
    // btpuWaitBinaryMatrixMul(BTPU0RegFile);
    // PRINTF_DBG("Storing results from BTPU...\n");
    // storeFramentsToBinaryMatrix(BTPU0_IO1_MEMORY, O, M, K);

    // success = true;
    // err = 0;
    // for(int i = 0; i < M * (K / 32); ++i){
    //     if(O[i] != result_Matrix[i]){
    //         if(success) PRINTF_DBG("Test #3-%d: FAILED -> O[%d] was 0x%08x, Expected = 0x%08x\n", i, i, O[i], result_Matrix[i]);
    //         success = false;
    //         ++err;
    //     }
    // }
    // if(success){
    //     PRINTF_DBG("Test #3: OK\n");
    // }else{
    //     PRINTF_DBG("   %d more errors found\n", err);
    //     PRINTF_DBG("Test #3: FAILED\n");
    // }

    BTPU0RegFile->creg.reg.BRAM_PORT_SEL = BTPU_BRAM_PORT_SEL_EXT;

    free(A);
    free(W);
    free(O);


    while(1){

    }

}
