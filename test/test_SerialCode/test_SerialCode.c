#include <main.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <BinaryMatMul.h>
#include <UART.h>
#include <GPIO.h>
#include <utilities.h>

#ifndef SIMULATION
    #define PRINTF_DBG(...) printf(__VA_ARGS__)
#else 
    #define PRINTF_DBG(...)
#endif

#define B_M 1
#define B_N 1
#define B_K 1

#define M (B_M * BINARY_FRAG_SIZE)
#define N (B_N * BINARY_FRAG_SIZE)
#define K (B_K * BINARY_FRAG_SIZE)

const int A_SIZE =  (M * (N / 32) * 4);
const int W_SIZE = (N * (K / 32) * 4);
const int O_BIN_SIZE = (M * (K / 32) * 4);
const int O_SIZE = (M * K * 4);
const int TOTAL_SIZE = A_SIZE + W_SIZE + O_BIN_SIZE;


uint32_t results[30];

BinaryAcc_t acc;

void test_BlockMatMul(int testN){
    //Popola le matrici A e W con valori di test
    for (int i = 0; i < N * (K / 32); ++i){
        BTPU0_W_MEMORY[0][i] = i;
    }
    
    for (int i = 0; i < M * (N / 32); ++i){
        BTPU0_IO0_MEMORY[0][i] = i + 1;
    }

    // Popola la matrice di output con zeri
    for (int i = 0; i < M * (K / 32); ++i){
        BTPU0_IO1_MEMORY[0][i] = 0;
    }

    fillAccWithZero(acc);
    fastBinaryBlockMatrixMul(BTPU0_IO0_MEMORY[0], BTPU0_W_MEMORY[0], acc, BTPU0_IO1_MEMORY[0], 30, true);

    // Calcola il risultato con la BTPU
    btpuSetAddrs(BTPU0RegFile, 0, 0, 1);
    btpuStartBinaryMatrixMul(BTPU0RegFile, 30, false, true, BTPU_USE_MEMORY_0_CONFIG);
    btpuWaitBinaryMatrixMul(BTPU0RegFile);

    int res = 1;
    int count = 0;
    // Verifica il risultato
    for (int i = 0; i < M * (K / 32); ++i){
        if (BTPU0_IO1_MEMORY[0][i] != BTPU0_IO1_MEMORY[1][i]){
            if(res) PRINTF_DBG("Test #%d-%d: FAILED -> BTPU0_IO1_MEMORY[0][%d] = %08X, expected %08X\n", testN, i, i, BTPU0_IO1_MEMORY[0][i], BTPU0_IO1_MEMORY[1][i]);
            res = 0;
            count++; 
        }
    }
    if(!res){
        PRINTF_DBG("    %d more like this...\n", count);
    }else{
        PRINTF_DBG("Test #%d: OK\n", testN);
    }
}

bool test_xnor32(int testN){
    uint32_t a = 0; 
    uint32_t b = 0;
    uint32_t result;
    result = xnor32(a, b);
    results[testN - 1] = result;
    if( result != 0xFFFFFFFF){
        PRINTF_DBG("Test #%d: FAILED -> xnor32(%08X, %08X) = %08X, expected %08X\n", testN, a, b, result, 0xFFFFFFFF);
        return false;
    } else {
        PRINTF_DBG("Test #%d: OK\n", testN);
        return true;
    }
}

bool test_popcount(int testN){
    uint32_t x = 0xFFFFFFFF;
    int result = popcount32(x);
    results[testN - 1] = result;
    if(result != 32){
        PRINTF_DBG("Test #%d: FAILED -> popcount32(%08X) = %d, expected %d\n", testN, x, result, 32);
        return false;
    } else {
        PRINTF_DBG("Test #%d: OK\n", testN);
        return true;
    }
}

bool test_binaryMul(int testN){
    uint32_t a = 0;
    uint32_t b = 0;
    uint32_t result = binaryMul(a, b);
    results[testN - 1] = result;
    if(result != 32){
        PRINTF_DBG("Test #%d: FAILED -> binaryMul(%08X, %08X) = %08X, expected %08X\n", testN, a, b, result, 32);
        return false;
    } else {
        PRINTF_DBG("Test #%d: OK\n", testN);
        return true;
    }
}



int main(int argc, char const *argv[]){

    int testN = 1;
    test_xnor32(testN);

    testN = 2;
    test_popcount(testN);

    testN = 3;
    test_binaryMul(testN);    

    #ifndef SIMULATION
    testN = 4;
    test_BlockMatMul(testN);

    // testN = 5;
    // test_MatrixMul(testN);
    #endif

    while(1);

}
