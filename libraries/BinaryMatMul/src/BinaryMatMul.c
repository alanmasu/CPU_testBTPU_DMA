#include "BinaryMatMul.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


// // Leggi un bit da una matrice binaria bit-packed
// uint8_t getBit(const BinaryMatrix_t mat, uint32_t row, uint32_t col) {
//     int bitIndex = row * SIZE + col;
//     int wordIndex = bitIndex / 32;
//     int bitPos = bitIndex % 32;
//     return (mat[wordIndex] >> bitPos) & 1;
// }

// // Scrivi un bit in una matrice binaria bit-packed
// void setBit(BinaryMatrix_t mat, uint32_t row, uint32_t col, uint8_t value) {
//     int bitIndex = row * SIZE + col;
//     int wordIndex = bitIndex / 32;
//     int bitPos = bitIndex % 32;
//     if (value)
//         mat[wordIndex] |= (1u << bitPos);
//     else
//         mat[wordIndex] &= ~(1u << bitPos);
// }

// // Trasponi una matrice binaria (da row-major a col-major)
// void transposeBinaryMatrix(const BinaryMatrix_t input, BinaryMatrix_t output) {
//     for (int i = 0; i < SIZE; i++) {
//         for (int j = 0; j < SIZE; j++) {
//             int bit = getBit(input, i, j);
//             setBit(output, j, i, bit);  // Trasposizione logica
//         }
//     }
// }