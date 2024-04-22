// main.h

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "address_map_arm.h"

#define DEBOUNCE_INTERVAL 200000 // microseconds

// Define a union for easier GPIO register mapping
typedef union {
    unsigned int value;
    struct {
        unsigned int gpio0 : 4;
        unsigned int gpio1 : 4;
        unsigned int gpio2 : 4;
        unsigned int gpio3 : 4;
        unsigned int gpio4 : 4;
        unsigned int gpio5 : 4;
        unsigned int gpiou : 11;
    } bits;
} GpioRegister;

//Union representing the switches
typedef union{
	unsigned int value;
	struct{
		unsigned int sw0:1;	//lsb-switch on the left
		unsigned int sw1:1;
		unsigned int sw2:1;
		unsigned int sw3:1;
		unsigned int sw4:1;
		unsigned int sw5:1;
		unsigned int sw6:1;
		unsigned int sw7:1;
		unsigned int sw8:1;
		unsigned int sw9:1;
	}bits;
}Switches;

extern void* LW_virtual;  // Shared memory base pointer

// Function declarations
int initialize_hardware();
void perform_cleanup();

#endif // MAIN_H
