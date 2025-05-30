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
    PRINTF_DBG("Hello World from test BTPU!\n");

    PRINTF_DBG("Starting allocation of matrices...\n");

    const int B_M = 2;
    const int B_N = 3;
    const int B_K = 4;
    const uint32_t signCmp = 48;

    const int M = B_M * BINARY_FRAG_SIZE;
    const int N = B_N * BINARY_FRAG_SIZE;
    const int K = B_K * BINARY_FRAG_SIZE;
 
    BinaryMatrix_t A1 = (BinaryMatrix_t)malloc(M * (N / 32) * sizeof(uint32_t)); //768 byte
    BinaryMatrix_t A2 = (BinaryMatrix_t)malloc(M * (N / 32) * sizeof(uint32_t)); //768 byte

    BinaryMatrix_t W = (BinaryMatrix_t)malloc(N * (K / 32) * sizeof(uint32_t)); // 1.536 kB

    BinaryMatrix_t O1 = (BinaryMatrix_t)malloc(M * (K / 32) * sizeof(uint32_t)); // 1 kB
    BinaryMatrix_t O2 = (BinaryMatrix_t)malloc(M * (K / 32) * sizeof(uint32_t)); // 1 kB

    BinaryMatrix_t O1Serial = (BinaryMatrix_t)malloc(M * (K / 32) * sizeof(uint32_t)); // 1 kB
    BinaryMatrix_t O2Serial = (BinaryMatrix_t)malloc(M * (K / 32) * sizeof(uint32_t)); // 1 kB

    if(!A1 || !A2 || !W || !O1 || !O2){
        PRINTF_DBG("Memory allocation failed!\n");
        while(1);
    }

    PRINTF_DBG("Memory allocation successful!\n");

    PRINTF_DBG("Initializing matrices...\n");
    for(int i = 0; i < M * (N / 32); ++i){
        A1[i] = i + 1;
        A2[i] = i + 2;
    }

    for(int i = 0; i < N * (K / 32); ++i){
        W[i] = i;
    }

    memset(O1, 0, M * (K / 32) * sizeof(uint32_t));
    memset(O2, 0, M * (K / 32) * sizeof(uint32_t));
    PRINTF_DBG("Matrices initialized successfully!\n");

    PRINTF_DBG("Loading matices to BTPU fragments...\n");
    loadBinaryMatrixToFragments(A1, BTPU0_IO0_MEMORY, M, N);
    loadBinaryMatrixToFragments(A2, BTPU0_IO1_MEMORY, M, N);
    loadBinaryMatrixToFragments(W, BTPU0_W_MEMORY, N, K);
    PRINTF_DBG("Matrices loaded to BTPU fragments successfully!\n");

    PRINTF_DBG("Starting first multiplication...\n");
    btpuSetBlocks(BTPU0RegFile, B_M, B_N, B_K);
    btpuSetAddrs(BTPU0RegFile, 0, 0, B_N * B_K); 
    btpuStartBinaryMatrixMul(BTPU0RegFile, signCmp, true, true, BTPU_OUT_MEMORY_0_CONFIG);

    PRINTF_DBG("Waiting for first multiplication to complete...\n");
    btpuWaitBinaryMatrixMul(BTPU0RegFile);
    PRINTF_DBG("First multiplication completed!\n");

    PRINTF_DBG("Storing results from BTPU fragments to O1 matrix...\n");
    storeFramentsToBinaryMatrix(BTPU0_IO0_MEMORY, O1, M, K);
    PRINTF_DBG("Results stored successfully!\n");

    PRINTF_DBG("Starting second multiplication...\n");
    btpuSetBlocks(BTPU0RegFile, B_M, B_N, B_K);
    btpuSetAddrs(BTPU0RegFile, 0, 0, B_N * B_K);
    btpuStartBinaryMatrixMul(BTPU0RegFile, signCmp, true, true, BTPU_OUT_MEMORY_0_CONFIG);

    PRINTF_DBG("Waiting for second multiplication to complete...\n");
    btpuWaitBinaryMatrixMul(BTPU0RegFile);
    PRINTF_DBG("Second multiplication completed!\n");

    PRINTF_DBG("Storing results from BTPU fragments to O2 matrix...\n");
    storeFramentsToBinaryMatrix(BTPU0_IO1_MEMORY, O2, M, K);
    PRINTF_DBG("Results stored successfully!\n");

    PRINTF_DBG("Performing first serial multiplication for verification...\n");
    fastBinaryMatrixMul(A1, W, O1Serial, M, N, K, signCmp);
    PRINTF_DBG("First serial multiplication completed!\n");
    PRINTF_DBG("Performing second serial multiplication for verification...\n");
    fastBinaryMatrixMul(A2, W, O2Serial, M, N, K, signCmp);
    PRINTF_DBG("Second serial multiplication completed!\n");

    PRINTF_DBG("Comparing results...\n");
    bool success = true;
    for(int i = 0; i < M * (K / 32); ++i){
        if(O1[i] != O1Serial[i]){
            PRINTF_DBG("Mismatch in O1 at index %d: BTPU = %u, Serial = %u\n", i, O1[i], O1Serial[i]);
            success = false;
        }
    }
    if(success){
        PRINTF_DBG("Test #1: OK\n");
    }else{
        PRINTF_DBG("Test #1: FAILED\n");
    }

    success = true;
    for(int i = 0; i < M * (K / 32); ++i){
        if(O2[i] != O2Serial[i]){
            PRINTF_DBG("Mismatch in O2 at index %d: BTPU = %u, Serial = %u\n", i, O2[i], O2Serial[i]);
            success = false;
        }
    }
    if(success){
        PRINTF_DBG("Test #2: OK\n");
    }else{
        PRINTF_DBG("Test #2: FAILED\n");
    }

    PRINTF_DBG("All tests completed!\n");
    PRINTF_DBG("Freeing allocated memory...\n");
    free(A1);
    free(A2);
    free(W);
    free(O1);
    free(O2);
    free(O1Serial);
    free(O2Serial);
    
    PRINTF_DBG("Setting BRAM Port to EXTERNAL...\n");
    BTPU0RegFile->creg.reg.BRAM_PORT_SEL = BTPU_BRAM_PORT_SEL_EXT; // Set BRAM port to external

    PRINTF_DBG("Trap CPU...\n");
    while(1);

    return 0;
}
