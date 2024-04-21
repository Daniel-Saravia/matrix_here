#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

// Define hardware register constants
#define HW_REGS_BASE (ALT_STM_OFST)     // Base address for hardware registers
#define HW_REGS_SPAN (0x04000000)       // Span size for hardware registers
#define HW_REGS_MASK (HW_REGS_SPAN - 1) // Mask for hardware registers

// Constants defining memory mappings and device specific details
#define LW_BRIDGE_BASE   (HW_REGS_BASE) // Base address for the Lightweight bridge
#define LW_BRIDGE_SPAN   (HW_REGS_SPAN) // Span size for the Lightweight bridge
#define HW_REGS_SPAN     // Redefine or ensure consistency in span size if required

// Push button register addresses
#define KEY_BASE        (0xFF200050) // Base address for key 3-0 data register
#define KEY_MASK        (0xFF200058) // Address for mask bits interrupt mask register
#define KEY_EDGE        (0xFF20005C) // Address for edge bits edge capture register

// Function prototypes
int initializeHardware(int* fd, void** virtual_base);
void readPushButtonState(volatile int* KEY_ptr);
void cleanup(void* virtual_base, int fd);

int main() {
    int fd;
    void* virtual_base;
    volatile int* KEY_ptr;

    // Initialize hardware and map memory
    if (initializeHardware(&fd, &virtual_base) != 0) {
        return 1; // Initialization failed
    }

    // Calculate the address of the Push Button Data Register
    KEY_ptr = (volatile int*)(virtual_base + (KEY_BASE - HW_REGS_BASE));

    // Read and print the state of the Push Buttons from the Data Register
    readPushButtonState(KEY_ptr);

    // Clean up resources and unmap memory
    cleanup(virtual_base, fd);

    return 0;
}

int initializeHardware(int* fd, void** virtual_base) {
    // Open the memory device
    *fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (*fd == -1) {
        printf("ERROR: could not open \"/dev/mem\"...\n");
        return 1;
    }

    // Map memory for the Lightweight bridge
    *virtual_base = mmap(NULL, LW_BRIDGE_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, *fd, LW_BRIDGE_BASE);
    if (*virtual_base == MAP_FAILED) {
        printf("ERROR: mmap() failed...\n");
        close(*fd);
        return 1;
    }

    return 0;
}

void readPushButtonState(volatile int* KEY_ptr) {
    printf("Keys are: %X\n", *KEY_ptr);
}

void cleanup(void* virtual_base, int fd) {
    if (munmap(virtual_base, HW_REGS_SPAN) != 0) {
        printf("ERROR: munmap() failed...\n");
    }
    close(fd);
}
