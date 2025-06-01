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


uint32_t a[M * (N / 32)];
uint32_t w[N * (K / 32)];

uint32_t oBin[M * (K / 32)];
// uint32_t o[M * K];

BinaryFragment_t a_frag;
BinaryFragment_t w_frag;
BinaryFragment_t o_frag;
BinaryAcc_t acc;

int main(int argc, char const *argv[]){
    for (int i = 0; i < N * (K / 32); ++i){
        w[i] = i;
    }
    
    for (int i = 0; i < M * (N / 32); ++i){
        a[i] = i + 1;
    }

    for (int i = 0; i < M * (K / 32); ++i){
        oBin[i] = 0;
    }

    PRINTF_DBG("Starting Binary Matrix Multiplication Test...\n");
    PRINTF_DBG("Test compiled at: %s %s\n", __DATE__, __TIME__);
    PRINTF_DBG("M = %d, N = %d, K = %d\n", M, N, K);

    loadFragment(a_frag, a, 0, 0, N);
    loadFragment(w_frag, w, 0, 0, K);

    fastBinaryBlockMatrixMul(a_frag, w_frag, acc, o_frag, 30, true);

    // binarizeMatrix((Matrix_t)acc, o_frag, 30, M, K);

    while(1);

}
