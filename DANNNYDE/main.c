// main.c

<<<<<<< HEAD
#include "main.h"

void* LW_virtual;  // Shared memory base pointer
=======
#define DEBOUNCE_INTERVAL 200000 // microseconds

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

void* LW_virtual;
int fd;  // Global variable for file descriptor to ensure it's accessible in all functions

// Function prototypes
int initialize_hardware(void);
void perform_cleanup(void);
>>>>>>> 278b2ed24af12814e81baf6604c6581f612185d7

int main() {
    if (initialize_hardware() == -1) {
        fprintf(stderr, "Failed to initialize hardware!\n");
        return -1;
    }

    volatile unsigned int* JP1_ptr = (volatile unsigned int*)(LW_virtual + JP1_BASE);
    *(JP1_ptr + 1) = 0x0000000F; // Set lower 4 bits for output
    *JP1_ptr = 0;  // Initialize the display to 0

    volatile signed int* SW_ptr = (volatile signed int*)(LW_virtual + SW_BASE);
    signed int previousSwitchState = *SW_ptr;

    printf("Flip switches to increment the display on the 7-segment decoder.\n");

    while (1) {
        signed int currentSwitchState = *SW_ptr;

        if (currentSwitchState != previousSwitchState) {
            unsigned int increment = 0;
            for (int i = 0; i < 10; i++) {
                if (currentSwitchState & (1 << i)) {
                    increment++;
                }
            }
            *JP1_ptr = (*JP1_ptr + increment) % 16; 
            printf("Displaying number %u on the 7-segment decoder circuits\n", *JP1_ptr);
            usleep(DEBOUNCE_INTERVAL);  

            previousSwitchState = currentSwitchState;
        }
    }

    perform_cleanup();
    return 0;
}

int initialize_hardware(void) {
    fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd == -1) {
        perror("ERROR: could not open \"/dev/mem\"");
        return -1;
    }

    LW_virtual = mmap(NULL, LW_BRIDGE_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, LW_BRIDGE_BASE);
    if (LW_virtual == MAP_FAILED) {
        perror("ERROR: mmap() failed");
        close(fd);
        return -1;
    }
    return 0;
}

void perform_cleanup(void) {
    if (LW_virtual) {
        munmap(LW_virtual, LW_BRIDGE_SPAN);
    }
    if (fd != -1) {
        close(fd);
    }
}
