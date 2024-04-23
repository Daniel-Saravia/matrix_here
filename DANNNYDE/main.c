#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "address_map_arm.h"

// Define constants
#define DEBOUNCE_INTERVAL 200000 // microseconds

// Global variables
int fd; // File descriptor for hardware access
void *LW_virtual;

// Function prototypes
int initialize_hardware(void);
void perform_cleanup(void);

// Union representing the switches
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sw0 : 1; // LSB-switch on the left
        unsigned int sw1 : 1;
        unsigned int sw2 : 1;
        unsigned int sw3 : 1;
        unsigned int unused : 6; // Adjusts for unused switches, assumes total 10 switches
    } bits;
} Switches;

int main()
{
    // Initialize hardware
    if (initialize_hardware() == -1)
    {
        fprintf(stderr, "Failed to initialize hardware!\n");
        return -1;
    }

    // Configure JP1 for 7-segment display output
    volatile unsigned int *JP1_ptr = (volatile unsigned int *)(LW_virtual + JP1_BASE);

    *(JP1_ptr + 1) = 0x0000000F; // Set lower 4 bits for output
    *JP1_ptr = 0;                // Reset display register to known state (0)

    // Set up button pointer
    volatile unsigned int *KEY_ptr = (volatile unsigned int *)(LW_virtual + KEY_BASE);
    unsigned int previousButtonState = *KEY_ptr;

    // Set up switch pointer
    volatile unsigned int *SW_ptr = (volatile unsigned int *)(LW_virtual + SW_BASE);
    unsigned int previousSwitchState = *SW_ptr; // Store initial switch state

    printf("Use switches SW0 to SW3 to input a binary number and display its decimal equivalent on the 7-segment display.\n");

    // Main loop to check switch changes and update display

    // init button state to 0 so 1 can be used to end program(key 0)
    unsigned int currentButtonState = 0;

    while (currentButtonState != 1) // (key 0)
    {
        unsigned int currentSwitchState = *SW_ptr & 0x0F; // Mask to ensure only the first four switches are read
        if (currentSwitchState != previousSwitchState)
        {
            *JP1_ptr = currentSwitchState; // Set the new state directly to the display
            // add display to SCREEN here
            printf("Displaying number: %u\n", (int)(*JP1_ptr - 4294967280));
            usleep(DEBOUNCE_INTERVAL);                // Debounce delay to prevent rapid state changes
            previousSwitchState = currentSwitchState; // Update previous state for comparison
        }
        currentButtonState = *KEY_ptr;
    }

    // Cleanup resources
    perform_cleanup();
    return 0;
}

int initialize_hardware()
{
    fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd == -1)
    {
        perror("ERROR: could not open \"/dev/mem\"");
        return -1;
    }

    LW_virtual = mmap(NULL, LW_BRIDGE_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, LW_BRIDGE_BASE);
    if (LW_virtual == MAP_FAILED)
    {
        perror("ERROR: mmap() failed");
        close(fd);
        return -1;
    }
    return 0;
}

void perform_cleanup()
{
    if (LW_virtual)
    {
        munmap(LW_virtual, LW_BRIDGE_SPAN);
    }
    if (fd != -1)
    {
        close(fd);
    }
}
