/*!
    @file       BinaryMatMul.h
    @brief      Header file for the BinaryMatMul library.
    @details    This file contains the structures and function prototypes for the BinaryMatMul library.
                The library is designed to perform binary matrix multiplication and will be optimized
                for the specific hardware architecture.

    @author     Alan Masutti  (@alanmasu)
    @date       14/05/2025
*/

#ifndef __BINARY_MATMUL_H__
#define __BINARY_MATMUL_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define BINARY_FRAG_SIZE 32


typedef uint32_t  BinaryFragment_t[BINARY_FRAG_SIZE];
typedef uint32_t* BinaryMatrix_t;
typedef uint32_t* Matrix_t;

typedef uint32_t  BinaryAcc_t[BINARY_FRAG_SIZE][BINARY_FRAG_SIZE];

/// Legge un bit da una matrice binaria bit-packed
uint8_t getBit(const BinaryMatrix_t mat, uint32_t row, uint32_t col, uint32_t N) {
    int bitIndex = row * N + col;
    int wordIndex = bitIndex / 32;
    int bitPos = bitIndex % 32;
    int bitShift = 31 - bitPos;
    uint8_t val = (mat[wordIndex] >> bitShift) & 1;
    return val;
}

/*!
    @brief Scrive un bit in una matrice binaria bit-packed
    @param mat La matrice binaria
    @param row La riga in cui scrivere il bit
    @param col La colonna in cui scrivere il bit
    @param value Il valore del bit da scrivere (0 o 1)
    @param N Il numero di colonne della matrice binaria (in bit)
*/
void setBit(BinaryMatrix_t mat, uint32_t row, uint32_t col, uint8_t value, uint32_t N) {
    int bitIndex = row * N + col;
    int wordIndex = bitIndex / 32;
    int bitPos = bitIndex % 32;
    int bitShift = 31 - bitPos;
    if (value){
        mat[wordIndex] |= (1u << bitShift);
    }else{
        mat[wordIndex] &= ~(1u << bitShift);
    }
}

/*!
    @brief  Traspone una matrice binaria (da row-major a col-major)
    @details Prende in ingresso una matrice binaria di dimensioni M x N (bit) e restituisce la sua trasposizione.
    @param[in]  input La matrice binaria di input
    @param[out] output La matrice binaria di output (trasposta)
    @param      M Numero di righe della matrice di input (in bit)
    @param      N Numero di colonne della matrice di input (in bit)
*/
void transposeBinaryMatrix(const BinaryMatrix_t input, BinaryMatrix_t output, const uint32_t M, const uint32_t N) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            int bit = getBit(input, i, j, N);
            setBit(output, j, i, bit, N);  // Trasposizione logica
        }
    }
}

/// Traspone un frammentp binario (da row-major a col-major)
void transposeBinaryFragment(BinaryFragment_t input, BinaryFragment_t output) {
    for (int i = 0; i < BINARY_FRAG_SIZE; i++) {
        for (int j = 0; j < BINARY_FRAG_SIZE; j++) {
            int bit = getBit(input, i, j, BINARY_FRAG_SIZE);
            setBit(output, j, i, bit, BINARY_FRAG_SIZE);  // Trasposizione logica
        }
    }
}

/// Calcola il numero di bit settati in una parola binaria
int popcount32(uint32_t x) {
    x = x - ((x >> 1) & 0x55555555);                    // put count of each 2 bits into those 2 bits
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);     // put count of each 4 bits into those 4 bits
    x = (x + (x >> 4)) & 0x0F0F0F0F;                    // put count of each 8 bits into those 8 bits
    x = x + (x >> 8);                                   // put count of each 16 bits into their lowest 8 bits
    x = x + (x >> 16);                                  // put count of each 32 bits into lowest 8 bits
    return x & 0x0000003F;
}

/// Esegue lo XNOR bitwise su due parole a 32 bit
uint32_t xnor32(const uint32_t a, const uint32_t b) {
    return ~(a ^ b);
}

/*!
    @brief  Moltiplica due parole binarie
    @details Prende in ingresso due parole binarie di 32 bit e restituisce il risultato della moltiplicazione
             come il numero di bit uguali tra le due parole.
    @param[in]  a La parola binaria A
    @param[in]  b La parola binaria B
    @return     Il numero di bit uguali tra le due parole
*/
uint32_t binaryMul(const uint32_t a, const uint32_t b) {
    uint32_t result;
    result = xnor32(a, b);
    result = popcount32(result);
    return result; 
}


/*!
    @brief  Moltiplica due frammenti di matrice binaria
    @details Prende in ingresso due frammenti di matrice binaria di dimensioni BINARY_FRAG_SIZE x BINARY_FRAG_SIZE
             e restituisce il risultato della moltiplicazione in un accumulatore.
    @param[in]  a Il frammento A
    @param[in]  b Il frammento B (trasposto)
    @param[out] acc L'accumulatore in cui memorizzare il risultato
*/
void binaryBlockMatrixMul(const BinaryFragment_t a, const BinaryFragment_t b, BinaryAcc_t acc, int i, int print) {
    for (int row = 0; row < BINARY_FRAG_SIZE; ++row) {
        for (int col = 0; col < BINARY_FRAG_SIZE; ++col) {
            acc[row][col] += binaryMul(a[row], b[col]);
            // if(row == 0 && col == 0 && print){
            if(row == 0 && col == 0 && print){
                printf("Iteration #%d: a[%d] = %d, b[%d] = %d, acc[%d][%d] = %d (%d)\n", i,      row, a[row], col, b[col], row, col, acc[row][col], binaryMul(a[row], b[col]));
                printf("               a[%d] = %08x, b[%d] = %08x, acc[%d][%d] = %08x (%08x)\n", row, a[row], col, b[col], row, col, acc[row][col], binaryMul(a[row], b[col]));
                printf("               a[%d] = %08x, b[%d] = %08x, a xnor b = %08x\n", row, a[row], col, b[col], xnor32(a[row], b[col]));
            }
        }
    }
}

/*!
    @brief  Carica un blocco di matrice binaria in un frammento
    @details Prende in ingresso una matrice binaria, allocata in RAM, e carica un frammento allocato in RAM
             con i dati del blocco specificato.
    @param[out] frag Il frammento in cui caricare i dati
    @param[in]  mat La matrice binaria da cui caricare i dati
    @param      blockRow L'indice di riga del blocco
    @param      blockCol L'indice di colonna del blocco
    @param      n Numero di colonne (in bit) della matrice binaria

*/
void loadFragment(BinaryFragment_t frag, const BinaryMatrix_t mat, uint32_t blockRow, uint32_t blockCol, uint32_t n) {
    const int cols = n / 32;
    int blockRowOffset = blockRow * cols * BINARY_FRAG_SIZE;
    int blockColOffset = blockCol;
    int wordRow = blockRowOffset + blockColOffset;
    for (int i = 0; i < BINARY_FRAG_SIZE; ++i) {
        frag[i] = mat[wordRow];
        wordRow += cols;
    }
}    

/*!
    @brief  Memorizza un frammento in una matrice binaria
    @details Prende in ingresso un frammento di matrice binaria e lo memorizza nella matrice binaria, 
             allocata in RAM, specificata nel blocco specificato.
    @param frag Il frammento da memorizzare
    @param mat La matrice binaria in cui memorizzare i dati
    @param blockRow L'indice di riga del blocco
    @param blockCol L'indice di colonna del blocco
    @param n Numero di colonne (in bit) della matrice binaria

*/
void storeFragment(const BinaryFragment_t frag, BinaryMatrix_t mat, uint32_t blockRow, uint32_t blockCol, uint32_t n) {
    const int cols = n / 32;
    int blockRowOffset = blockRow * cols * BINARY_FRAG_SIZE;
    int blockColOffset = blockCol;
    int wordRow = blockRowOffset + blockColOffset;
    for (int i = 0; i < BINARY_FRAG_SIZE; ++i) {
        mat[wordRow] = frag[i];
        wordRow += cols;
    }
}

/*!
    @brief  Memorizza un accumulatore in una matrice
    @details Prende in ingresso un accumulatore di dimensioni BINARY_FRAG_SIZE x BINARY_FRAG_SIZE
             e lo memorizza nella matrice, allocata in RAM, specificata nel blocco specificato.
    @param acc L'accumulatore da memorizzare
    @param mat La matrice in cui memorizzare i dati
    @param blockRow L'indice di riga del blocco
    @param blockCol L'indice di colonna del blocco
    @param n Numero di colonne della matrice
*/
void storeAcc(const BinaryAcc_t acc, Matrix_t mat, uint32_t blockRow, uint32_t blockCol, uint32_t n) {
    int blockRowOffset = blockRow * BINARY_FRAG_SIZE * n;
    int blockColOffset = blockCol * BINARY_FRAG_SIZE;
    for (int row = 0; row < BINARY_FRAG_SIZE; ++row) {
        for (int col = 0; col < BINARY_FRAG_SIZE; ++col) {
            mat[blockRowOffset + row * n + blockColOffset + col] = acc[row][col];
        }
    }
}

/*!
    @brief  Inizializza un accumulatore a zero
    @param acc L'accumulatore da inizializzare

*/
void fillAccWithZero(BinaryAcc_t acc) {
    for(int row = 0; row < BINARY_FRAG_SIZE; ++row){
        for(int col = 0; col < BINARY_FRAG_SIZE; ++col){
            acc[row][col] = 0;
        }
    }
}

/*!
    @brief      Moltiplica due matrici binarie
    @details    Prende in ingresso due matrici binarie di dimensioni m x n e n x k
                e restituisce il risultato della moltiplicazione in una matrice di dimensioni m x k.
                La moltiplicazione viene eseguita in blocchi di dimensione BINARY_FRAG_SIZE x BINARY_FRAG_SIZE.
    @param[in]  a La matrice binaria A
    @param[in]  b La matrice binaria B
    @param[out] result La matrice risultante
    @param      m Numero di righe della matrice A
    @param      n Numero di colonne della matrice A e righe della matrice B
    @param      k Numero di colonne della matrice B
*/
void binaryMatrixMul(const BinaryMatrix_t a, const BinaryMatrix_t b, Matrix_t result, const int m, const int n, const int k) {
    uint32_t blockM = m / BINARY_FRAG_SIZE;
    uint32_t blockN = n / BINARY_FRAG_SIZE;
    uint32_t blockK = k / BINARY_FRAG_SIZE;
    BinaryFragment_t a_frag;
    BinaryFragment_t b_frag;
    BinaryFragment_t b_transposed;
    BinaryAcc_t acc;
    for(int blockRow = 0; blockRow < blockM; ++blockRow){
        for (int blockCol = 0; blockCol < blockK; ++blockCol) {
            fillAccWithZero(acc);  
            for (int i = 0; i < blockN; ++i) {
                loadFragment(a_frag, a, blockRow, i, n);
                loadFragment(b_frag, b, i, blockCol, k);
                transposeBinaryFragment(b_frag, b_transposed);
                // printf("Block: [%d][%d] i: %d\n", blockRow, blockCol, i);
                binaryBlockMatrixMul(a_frag, b_transposed, acc, i, 1);
                printf("\n");
            }
            storeAcc(acc, result, blockRow, blockCol, k);
        }
    }
}

/*!
    @brief  Converte una matrice in una matrice binaria

    @param[in]  mat La matrice da convertire
    @param[out] bMat La matrice binaria risultante
    @param      signCmp Il valore di confronto per la binarizzazione
    @param      m Numero di righe della matrice (in bit)
    @param      n Numero di colonne della matrice (in bit)
*/
void binarizeMatrix(Matrix_t mat, BinaryMatrix_t bMat, uint32_t signCmp, uint32_t m, uint32_t n){
    for (int i = 0; i < m; ++i){
        for (int j = 0; j < n; ++j){
            setBit(bMat, i, j, mat[i * n + j] > signCmp, n);
        }
    }
}

/*!
    @brief  Stampa una matrice binaria come interi

    @param mat La matrice binaria da stampare
    @param r Numero di righe da stampare
    @param c Numero di colonne da stampare
    @param M Numero di righe della matrice (in bit)
    @param N Numero di colonne della matrice (in bit)
*/
void printIntBMatrixN(BinaryMatrix_t mat, uint32_t r, uint32_t c, const uint32_t M, const uint32_t N){
    int cols = N / 32;
    if (c > cols){
        c = cols;
    }
    if (r > M){
        r = M;
    }

    for (int i = 0; i < r; ++i){
        for (int j = 0; j < c; ++j){
            printf("%03d ", *(mat + i * (N/32) + j));
        }
        printf("\n");
    }
}

/*!
    @brief  Stampa una matrice binaria

    @param mat La matrice binaria da stampare
    @param r Numero di righe da stampare
    @param c Numero di colonne da stampare
    @param M Numero di righe della matrice (in bit)
    @param N Numero di colonne della matrice (in bit)

*/
void printBMatrixN(BinaryMatrix_t mat, uint32_t r, uint32_t c, const uint32_t M, const uint32_t N){
    if (c > N){
        c = N;
    }
    if (r > M){
        r = M;
    }
    for(int row = 0; row < r; ++row){
        for(int col = 0; col < c; ++col){
            uint8_t bit = getBit(mat, row, col, N);
            printf("%d ", bit);
        }
        printf("\n");
    }
}
/*!
    @brief  Stampa un frammento binario come interi

    @param frag Il frammento da stampare
    @param rows Numero di righe da stampare
*/
void printIntFragmentN(BinaryFragment_t frag, uint32_t rows){
    if (rows > BINARY_FRAG_SIZE){
        rows = BINARY_FRAG_SIZE;
    }
    for (int i = 0; i < rows; ++i){
        printf("%03d\n", frag[i]);
    }
    printf("\n");
}

/*!
    @brief  Stampa una matrice

    @param mat La matrice da stampare
    @param r Numero di righe da stampare
    @param c Numero di colonne da stampare
    @param M Numero di righe della matrice
    @param N Numero di colonne della matrice
*/
void printIntMatrixN(Matrix_t mat, uint32_t r, uint32_t c, const uint32_t M, const uint32_t N){
    if (c > N){
        c = N;
    }
    if (r > M){
        r = N;
    }

    for (int i = 0; i < r; ++i){
        for (int j = 0; j < c; ++j){
            printf("%03d ", *(mat + i * N + j));
        }
        printf("\n");
    }
}

#endif
