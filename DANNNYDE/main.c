#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#define _DEFAULT_SOURCE
#include <unistd.h>
#include <sys/mman.h>
#include "address_map_arm.h"

// Define a union for easier GPIO register mapping.
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

// Global variables
int fd = -1; // File descriptor for accessing /dev/mem
void* LW_virtual; // Virtual base address after mapping

// Function prototypes
int open_physical(int fd);
void close_physical(int fd);
void* map_physical(int fd, unsigned int base, unsigned int span);
int unmap_physical(void* virtual_base, unsigned int span);
int initialize_hardware();

// Function implementations
int open_physical(int fd) {
    if (fd == -1) {
        fd = open("/dev/mem", (O_RDWR | O_SYNC));
        if (fd == -1) {
            perror("ERROR: could not open \"/dev/mem\"");
            return -1;
        }
    }
    return fd;
}

void close_physical(int fd) {
    close(fd);
}

void* map_physical(int fd, unsigned int base, unsigned int span) {
    void *virtual_base = mmap(NULL, span, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, base);
    if (virtual_base == MAP_FAILED) {
        perror("ERROR: mmap() failed");
        close(fd);
        return NULL;
    }
    return virtual_base;
}

int unmap_physical(void* virtual_base, unsigned int span) {
    if (munmap(virtual_base, span) != 0) {
        perror("ERROR: munmap() failed");
        return -1;
    }
    return 0;
}

int initialize_hardware() {
    // Attempt to open and map the necessary physical memory
    if ((fd = open_physical(fd)) == -1) {
        return -1;
    }
    if ((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL) {
        close_physical(fd);
        return -1;
    }
    return 0;
}

int main() {
    if (initialize_hardware() == -1) {
        fprintf(stderr, "Failed to initialize hardware!\n");
        return -1;
    }

    // Configure a specific port (JP1) for output
    volatile unsigned int* JP1_ptr = (unsigned int*)(LW_virtual + JP1_BASE);
    *(JP1_ptr + 1) = 0x0000000F; // Set lower 4 bits for output

    *JP1_ptr = 7; // Set the output to display the number "6".

    printf("Displaying number 7 on the 7 Segment Decoder circuits\n");

    // Perform cleanup by unmapping memory and closing the file descriptor
    unmap_physical(LW_virtual, LW_BRIDGE_SPAN);
    close_physical(fd);
    return 0;
}