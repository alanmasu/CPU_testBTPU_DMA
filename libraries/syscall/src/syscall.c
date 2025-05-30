#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <UART.h>

extern uint8_t __heap_start;
static uint8_t *heap_ptr = &__heap_start;

static uint8_t* ddr_heap_ptr;
static uint8_t* __ddr_heap_start;
static uint32_t ddr_heap_size;

void *_sbrk(int incr) {
    uint32_t __stack_pointer;
    __asm__ volatile (
        "mv %0, sp"
        : "=r"(__stack_pointer)
    );

    uint8_t* prev;
    if (incr > 0){ // allocazione di memoria
        uint8_t ddrHeapSize = ddr_heap_ptr + incr - __ddr_heap_start;
        if (heap_ptr + incr < (uint8_t*)__stack_pointer) {
            prev = heap_ptr;
            heap_ptr += incr;
        }else if(ddr_heap_ptr != NULL && ddrHeapSize < ddr_heap_size){
            prev = ddr_heap_ptr;
            ddr_heap_ptr += incr;
        }else{
            errno = ENOMEM;
            return (void *)-1;
        }
    }else{        // deallocazione di memoria
        if(ddr_heap_ptr > __ddr_heap_start){
            prev = ddr_heap_ptr;
            ddr_heap_ptr += incr;
        }

    }
    return (void *)prev;
}

int _write(int fd, const void *buf, size_t count) {
    // Inserisci qui codice per scrivere su UART o simile
    UARTWrite((uint8_t*)buf, count);
    return count;
}

int _read(int fd, void *buf, size_t count) {
    // Inserisci qui codice per leggere da UART o simile
    return UARTRead((uint8_t*)buf, count, UART_READ_BLOCKING);
}

void _putchar(char character){
    UARTWrite((uint8_t*)&character, 1);
}

int _close(int fd){
    return -1; 
}

int _fstat(int fd, struct stat *st) { 
    st->st_mode = S_IFCHR; 
    return 0; 
}
int _lseek(int fd, int ptr, int dir) { 
    return 0; 
}
int _isatty(int fd){ 
    return 1; 
}
