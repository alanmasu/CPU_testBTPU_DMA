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

#define B_M 2
#define B_N 3
#define B_K 4
#define SIGN_CMP 48

#define M (B_M * BINARY_FRAG_SIZE)
#define N (B_N * BINARY_FRAG_SIZE)
#define K (B_K * BINARY_FRAG_SIZE)

const int A_SIZE =  (M * (N / 32) * 4);
const int W_SIZE = (N * (K / 32) * 4);
const int O_BIN_SIZE = (M * (K / 32) * 4);
const int O_SIZE = (M * K * 4);
const int TOTAL_SIZE = A_SIZE + W_SIZE + O_BIN_SIZE;

int WMemoryBlocksUsed = 0;
int IO0MemoryBlocksUsed = 0;
int IO1MemoryBlocksUsed = 0;


uint32_t results[30];

BinaryAcc_t acc;

void test_BlockMatMul(int testN){
    WMemoryBlocksUsed += 1;
    IO0MemoryBlocksUsed += 1;
    IO1MemoryBlocksUsed += 3;

    // Controlla l'allocazione della memoria
    if (IO0MemoryBlocksUsed > BTPU_MAX_BLOCK_COUNT || 
        WMemoryBlocksUsed > BTPU_MAX_BLOCK_COUNT || 
        IO1MemoryBlocksUsed > BTPU_MAX_BLOCK_COUNT) {
        PRINTF_DBG("Memory allocation error: too many blocks used.\n");
        return;
    }

    //Popola le matrici A e W con valori di test
    for (int i = 0; i < BINARY_FRAG_SIZE; ++i){
        BTPU0_W_MEMORY[0][i] = i;
    }
    
    for (int i = 0; i < BINARY_FRAG_SIZE; ++i){
        BTPU0_IO0_MEMORY[0][i] = i + 1;
    }

    // Popola la matrice di output con zeri
    for (int i = 0; i < BINARY_FRAG_SIZE; ++i){
        BTPU0_IO1_MEMORY[0][i] = 0;
    }

    fillAccWithZero(acc);
    fastBinaryBlockMatrixMul(BTPU0_IO0_MEMORY[0], BTPU0_W_MEMORY[0], acc, BTPU0_IO1_MEMORY[0], 30, true);

    binarizeMatrix(BTPU0_IO1_MEMORY[0], BTPU0_IO1_MEMORY[2], 30, M, K);

    // Calcola il risultato con la BTPU
    btpuSetAddrs(BTPU0RegFile, WMemoryBlocksUsed - 1, IO0MemoryBlocksUsed - 1, IO1MemoryBlocksUsed - 1);
    btpuStartBinaryMatrixMul(BTPU0RegFile, 30, false, true, BTPU_USE_MEMORY_0_CONFIG);
    btpuWaitBinaryMatrixMul(BTPU0RegFile);

    int res = 1;
    int count = 0;
    int inTestN = 1;
    // Verifica il risultato fast con la BTPU
    for (int i = 0; i < BINARY_FRAG_SIZE; ++i){
        if (BTPU0_IO1_MEMORY[0][i] != BTPU0_IO1_MEMORY[2][i]){
            if(res) PRINTF_DBG("Test #%d-%d: FAILED -> BTPU0_IO1_MEMORY[0][%d] = %08X, expected %08X\n", testN, inTestN, i, BTPU0_IO1_MEMORY[0][i], BTPU0_IO1_MEMORY[1][i]);
            res = 0;
            count++; 
        }
    }
    if(!res){
        PRINTF_DBG("    %d more like this...\n", count);
    }else{
        PRINTF_DBG("Test #%d-%d: OK\n", testN, inTestN);
    }

    inTestN = 2;
    // Verifica il risultato con la BTPU
    for (int i = 0; i < BINARY_FRAG_SIZE; ++i){
        if (BTPU0_IO1_MEMORY[1][i] != BTPU0_IO1_MEMORY[2][i]){
            if(res) PRINTF_DBG("Test #%d-%d: FAILED -> BTPU0_IO1_MEMORY[1][%d] = %08X, expected %08X\n", testN, inTestN, i, BTPU0_IO1_MEMORY[1][i], BTPU0_IO1_MEMORY[2][i]);
            res = 0;
            count++; 
        }
    }
    if(!res){
        PRINTF_DBG("    %d more like this...\n", count);
    }else{
        PRINTF_DBG("Test #%d-%d: OK\n", testN, inTestN);
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

void test_MatrixMul(int testN){
    BinaryMatrix_t aSerial = (BinaryMatrix_t)(IO0_MEMORY_BASE) + 32 * IO0MemoryBlocksUsed;
    IO0MemoryBlocksUsed += B_M * B_N;
    BinaryMatrix_t wSerial = (BinaryMatrix_t)(W_MEMORY_BASE)   + 32 * WMemoryBlocksUsed;
    WMemoryBlocksUsed   += B_N * B_K;
    BinaryMatrix_t oSerial = (BinaryMatrix_t)(IO1_MEMORY_BASE) + 32 * IO1MemoryBlocksUsed;
    IO1MemoryBlocksUsed += B_M * B_K;
    BinaryMatrix_t oBTPU = (BinaryMatrix_t)(IO1_MEMORY_BASE) + 32 * IO1MemoryBlocksUsed;
    IO1MemoryBlocksUsed += B_M * B_K;

    //Allocazione della memoria per la computazione con BTPU
    IO0MemoryBlocksUsed += B_M * B_N;
    WMemoryBlocksUsed   += B_N * B_K;
    IO1MemoryBlocksUsed += B_M * B_K;

    // Controlla l'allocazione della memoria
    if (IO0MemoryBlocksUsed >= BTPU_MAX_BLOCK_COUNT || 
        WMemoryBlocksUsed >= BTPU_MAX_BLOCK_COUNT || 
        IO1MemoryBlocksUsed >= BTPU_MAX_BLOCK_COUNT) {
        PRINTF_DBG("Memory allocation error: too many blocks used.\n");
        // return;
    }

    // Inizializza le matrici A e W con valori di test
    for (int i = 0; i < M * (N / 32); ++i){
        aSerial[i] = i + 1;
    }
    for (int i = 0; i < N * (K / 32); ++i){
        wSerial[i] = i;
    }
    // Inizializza la matrice di output con zeri
    for (int i = 0; i < M * (K / 32); ++i){
        oSerial[i] = 0;
    }


    fastBinaryMatrixMul(aSerial, wSerial, oSerial, SIGN_CMP, M, N, K);

    // Calcola il risultato con la BTPU
    // Carica le matrici A e W nella memoria della BTPU
    loadBinaryMatrixToFragments(aSerial, BTPU0_IO0_MEMORY + IO0MemoryBlocksUsed - B_M * B_N, M, N);
    loadBinaryMatrixToFragments(wSerial, BTPU0_W_MEMORY + WMemoryBlocksUsed - B_N * B_K, N, K);
    // Configura la BTPU per la moltiplicazione di matrici
    btpuSetAddrs(BTPU0RegFile, WMemoryBlocksUsed - B_N * B_K, IO0MemoryBlocksUsed - B_M * B_N, IO1MemoryBlocksUsed - B_M * B_K);
    btpuSetBlocks(BTPU0RegFile, B_M, B_N, B_K);
    btpuStartBinaryMatrixMul(BTPU0RegFile, SIGN_CMP, true, true, BTPU_USE_MEMORY_0_CONFIG);
    // Attende il completamento della moltiplicazione
    btpuWaitBinaryMatrixMul(BTPU0RegFile);
    // Memorizza il risultato della BTPU nella matrice di output
    storeFramentsToBinaryMatrix(BTPU0_IO1_MEMORY + IO1MemoryBlocksUsed - B_M * B_K, oBTPU, M, K);

    int res = 1;
    int count = 0;
    // Verifica il risultato fast con la BTPU
    for (int i = 0; i < M * (K / 32); ++i){
        if (oSerial[i] != oBTPU[i]){
            if(res) PRINTF_DBG("Test #%d: FAILED -> oSerial[%d] = %08X, expected %08X\n", testN, i, oSerial[i], oBTPU[i]);
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

        testN = 5;
        test_MatrixMul(testN);
    #endif

    while(1);

}
