#include <main.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <BinaryMatMul.h>
#include <UART.h>

#define SIMULATION

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

    BinaryMatrix_t A1 = (BinaryMatrix_t)malloc(M * (N / 32) * sizeof(uint32_t));
    BinaryMatrix_t A2 = (BinaryMatrix_t)malloc(M * (N / 32) * sizeof(uint32_t));

    BinaryMatrix_t W = (BinaryMatrix_t)malloc(N * (K / 32) * sizeof(uint32_t));

    BinaryMatrix_t O1 = (BinaryMatrix_t)malloc(M * (K / 32) * sizeof(uint32_t));
    BinaryMatrix_t O2 = (BinaryMatrix_t)malloc(M * (K / 32) * sizeof(uint32_t));

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
    btpuStartBinaryMatrixMul(BTPU0RegFile, signCmp, true, BTPU_OUT_MEMORY_0_CONFIG);

    PRINTF_DBG("Waiting for first multiplication to complete...\n");
    btpuWaitBinaryMatrixMul(BTPU0RegFile);
    PRINTF_DBG("First multiplication completed!\n");
    

    return 0;
}
