/**!
 * @file    UART.c
 * 
 * @author  Alan Masutti (@alanmasu)
 * @date    30/03/2025
 * 
*/
#include <UART.h>

#include <stdio.h>
#include <stdint.h>
#include <errno.h>

void UARTInit (void){
    volatile uint32_t* ieReg = (uint32_t*)(UART_IE_ADDR);
    volatile uint32_t* imReg = (uint32_t*)(UART_IM_ADDR);
    printf("UARTInit called: imReg = %p\n", *imReg);
    *ieReg |= UART_RXTO_I;
    printf("UARTInit set: imReg = %p\n", *imReg);

}

void UARTWrite (const uint8_t* data, uint32_t size){
    volatile uint32_t* statusReg = (uint32_t*)(UART_STATUS_ADDR);
    volatile uint8_t* fifoReg = (uint8_t*)(UART_FIFO_ADDR);
    if (size == 0){
        return;
    }
    if (data == NULL){
        return;
    }
    while(!(*statusReg & UART_TXEMPTY)); // wait until the TX FIFO is empty
    for (uint32_t i = 0; i < size; ++i){
        (*fifoReg) = data[i];
        while((*statusReg & UART_TXFULL)); // wait until the TX FIFO is full
    }
}


uint16_t UARTRead (uint8_t* data, uint32_t size, UARTReadMode_t mode) {
    volatile uint32_t* statusReg    = (uint32_t*)(UART_STATUS_ADDR);
    volatile uint8_t*  fifoReg      = (uint8_t*) (UART_FIFO_ADDR);
    volatile uint8_t*  rtrigReg     = (uint8_t*) (UART_RTRIG_ADDR);
    volatile uint32_t* rxTimeoutReg = (uint32_t*)(UART_RXTO_ADDR);
    volatile uint32_t* iStatusReg   = (uint32_t*)(UART_INT_STS_ADDR);

    // printf("UARTRead called with: %d bytes\n", size);

    if (size == 0 || data == NULL) {
        errno = EINVAL;
        printf("UARTRead returned: EINVAL\n");
        return -1;
    }

    uint32_t totalRead = 0;

    while (totalRead < size) {
        uint32_t chunkSize = size - totalRead;

        if (chunkSize > 63) {
            chunkSize = 63; // Al massimo 63 byte per volta
        }

        *rtrigReg = (mode == UART_READ_NONBLOCKING) ? 63 : chunkSize;
        *rxTimeoutReg = (mode == UART_READ_NONBLOCKING) ? 255 : 0;

        uint32_t i = 0;
        while (i < chunkSize) {
            while ((*statusReg & UART_RXEMPTY)) {
                if (mode == UART_READ_NONBLOCKING) {
                    if (*iStatusReg & UART_RXTO_I) {
                        errno = EAGAIN;
                        // printf("UARTRead returned: EAGAIN -> readed: %d bytes\n", totalRead + i);
                        *iStatusReg = 0;
                        return totalRead + i; // ritorna quanto letto fino ad ora
                    }
                }
                // in blocking mode aspettiamo sempre
            }

            // Ora almeno un byte disponibile
            data[totalRead + i] = *fifoReg;
            i++;
        }

        totalRead += i;

        // Se siamo in modalità non bloccante, leggiamo solo una volta!
        if (mode == UART_READ_NONBLOCKING) {
            break;
        }
    }
    printf("UARTRead returned: %d bytes\n", totalRead);
    return totalRead;
}

void UARTPrint(const char* str){
    while(*str){
        UARTWrite((uint8_t*)str, 1);
        ++str;
    }
}

