#include <main.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <BinaryMatMul.h>
#include <UART.h>
#include <GPIO.h>
#include <utilities.h>
#include <OLED.h>

// __attribute__((optimize("O0")))
void initMatrices(){
    for(int i = 0; i < BINARY_FRAG_SIZE; ++i){
        BTPU0_W_MEMORY[0][i] = i;
        BTPU0_IO0_MEMORY[0][i] = i + 1;
        BTPU0_IO1_MEMORY[0][i] = i + 2;
    }
}

int main(int argc, char const *argv[]){

    GPIO0Dir->PORT_A_DIR.GPIO0 = OUTPUT; // GPIO0_PIN0 (LED0)
    GPIO0Dir->PORT_A_DIR.GPIO1 = OUTPUT; // GPIO0_PIN1 (LED1)
    GPIO0Dir->PORT_A_DIR.GPIO2 = OUTPUT; // GPIO0_PIN2 (LED2)
    GPIO0Dir->PORT_A_DIR.GPIO3 = OUTPUT; // GPIO0_PIN3 (LED3)

    // Initialize Matrices
    initMatrices();

    //Configure first Binary Matrix Mul 
    btpuSetAddrs(BTPU0RegFile, 0, 0, 1);
    // btpuSetBlocks(BTPU0RegFile, 1, 1, 1);
    // BTPU0RegFile->nSize = 1;
    btpuStartBinaryMatrixMul(BTPU0RegFile, 30, false, true, BTPU_USE_MEMORY_0_CONFIG);

    btpuWaitBinaryMatrixMul(BTPU0RegFile);

    // //Configure second Binary Matrix Mul
    // btpuSetAddrs(BTPU0RegFile, 0, 0, 1);
    // btpuStartBinaryMatrixMul(BTPU0RegFile, 30, false, true, BTPU_USE_MEMORY_1_CONFIG);

    // btpuWaitBinaryMatrixMul(BTPU0RegFile);

    BTPU0RegFile->creg.reg.BRAM_PORT_SEL = BTPU_BRAM_PORT_SEL_EXT;
#ifndef SIMULATION
    printf("BTPU CREG:\n");
    printf("     start: %d\n", BTPU0RegFile->creg.reg.START);
    printf("     busy: %d\n", BTPU0RegFile->creg.reg.BUSY);
    printf("     omem_sel: %d\n", BTPU0RegFile->creg.reg.OMEM_SEL);
    printf("     bram_port_sel: %d\n", BTPU0RegFile->creg.reg.BRAM_PORT_SEL);
    printf("     acc_clear: %d\n", BTPU0RegFile->creg.reg.ACC_CLEAR);
    printf("     batched_mul: %d\n", BTPU0RegFile->creg.reg.BATCHED_MUL);
    printf("     error: %d\n", BTPU0RegFile->creg.reg.ERROR);
    printf("\n");
    printf("BTPU W Addr: %d\n", BTPU0RegFile->wMemStartAddr);
    printf("BTPU I Addr: %d\n", BTPU0RegFile->iMemStartAddr);
    printf("BTPU O Addr: %d\n", BTPU0RegFile->oMemStartAddr);
    printf("BTPU signCmp: %d\n", BTPU0RegFile->signCmp);
#endif

    int i = 0;
    // Trap CPU
    while(1){
        // // Read GPIO0_PIN0 and GPIO0_PIN1 to select data to show on OLED
        // GPIO0Data->PORT_A_DATA.GPIO0 = controlRegister->IO.sw0; // GPIO0_PIN0
        // GPIO0Data->PORT_A_DATA.GPIO1 = controlRegister->IO.sw1; // GPIO0_PIN1

        // OLED0Data->data[0] = *((uint8_t*)GPIO0Port) & 0xFF;
        // OLED0Data->data[1] = controlRegister->IO.sw0 | (controlRegister->IO.sw1 << 1);

        
        // if (i == DELAY_COUNT / 2) {
        //     i = 0;
        //     GPIO0Data->PORT_A_DATA.GPIO3 = !GPIO0Port->PORT_A.GPIO3; // Toggle GPIO0_PIN6 (LED)
        //     printf("GPIO0 PORT A: %02x\n", GPIO0Port->PORT_A);
        // } 
        // i = i + 1;
       
    }
}
