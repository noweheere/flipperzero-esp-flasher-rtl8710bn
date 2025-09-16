// SLIP (Serial Line Internet Protocol) implementation for ESP flasher

#include <stdint.h>
#include <string.h>

#define SLIP_END     0xC0
#define SLIP_ESC     0xDB
#define SLIP_ESC_END 0xDC
#define SLIP_ESC_ESC 0xDD

int slip_encode(uint8_t* dst, const uint8_t* src, int size) {
    int j = 0;
    dst[j++] = SLIP_END;
    
    for(int i = 0; i < size; i++) {
        if(src[i] == SLIP_END) {
            dst[j++] = SLIP_ESC;
            dst[j++] = SLIP_ESC_END;
        } else if(src[i] == SLIP_ESC) {
            dst[j++] = SLIP_ESC;
            dst[j++] = SLIP_ESC_ESC;
        } else {
            dst[j++] = src[i];
        }
    }
    
    dst[j++] = SLIP_END;
    return j;
}

int slip_decode(uint8_t* dst, const uint8_t* src, int size) {
    int i = 0, j = 0;
    bool in_frame = false;
    
    while(i < size) {
        if(src[i] == SLIP_END) {
            if(in_frame) {
                return j; // End of frame
            } else {
                in_frame = true; // Start of frame
            }
        } else if(src[i] == SLIP_ESC && i + 1 < size) {
            i++;
            if(src[i] == SLIP_ESC_END) {
                dst[j++] = SLIP_END;
            } else if(src[i] == SLIP_ESC_ESC) {
                dst[j++] = SLIP_ESC;
            }
        } else if(in_frame) {
            dst[j++] = src[i];
        }
        i++;
    }
    
    return j;
}