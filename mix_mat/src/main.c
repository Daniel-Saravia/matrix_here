#define _DEFAULT_SOURCE

#include "terasic_os_includes.h"
#include "LCD_Lib.h"
#include "lcd_graphic.h"
#include "font.h"
#include "gameLogic.h"
#include "address_map_arm.h"

// Define hardware register constants
#define HW_REGS_BASE (ALT_STM_OFST)
#define HW_REGS_SPAN (0x04000000)
#define HW_REGS_MASK (HW_REGS_SPAN - 1)
#define DEBOUNCE_INTERVAL 500000 // microseconds

#define BUTTON_EXIT 1       // Used to exit the program or a loop
#define BUTTON_ADD_NUMBER 2 // Triggers a specific action, like starting an operation
#define BUTTON_OTHER1 3     // Reserved for future use or another specific function
#define BUTTON_OTHER2 4     // Reserved for future use or another specific function

// Global varibles
int fd;
void *LW_virtual;

// Function prototypes
int initialize_hardware(void);
void perform_cleanup(void);
int openMemoryDevice();
void *mapMemory(int fd);
void initializeLCDCanvas(LCD_CANVAS *canvas, void *virtual_base);
void drawGrid(LCD_CANVAS *canvas);
void print_matrix_multiplication(LCD_CANVAS *canvas, int *result);
void cleanup(void *virtual_base, int fd, LCD_CANVAS *canvas);
void clearNumbers(LCD_CANVAS *canvas);
void displayGridOnLCD(LCD_CANVAS *canvas);
void matrix_multiplication(int *result, int *matrixA, int *matrixB);
void storeMatrixValues(int switches_input, int table_index, int *matrixA, int *matrixB);
void printNumberOnLCD(LCD_CANVAS *canvas, int switches_input, int table_index);

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

// Main function - entry point of the program
int main()
{
    LCD_CANVAS LcdCanvas;

    // matrix_multiplication varibles
    int matrixA[4]; // Entries for the first matrix
    int matrixB[4]; // Entries for the second matrix
    int result[4];

    // Initialize hardware
    if (initialize_hardware() == -1)
    {
        fprintf(stderr, "Failed to initialize hardware!\n");
        return -1;
    }

    ////////SCREEN Map hardware registers into user space
    void *virtual_base = mapMemory(fd);
    if (virtual_base == MAP_FAILED)
        return 1; // Exit if mapping fails
    ////////END SCREEN Map hardware registers into user space

    // Configure JP1 for 7-segment display output
    volatile unsigned int *JP1_ptr = (volatile unsigned int *)(LW_virtual + JP1_BASE);

    *(JP1_ptr + 1) = 0x0000000F; // Set lower 4 bits for output
    *JP1_ptr = 0;                // Reset display register to known state (0)

    // Set up button pointer
    volatile unsigned int *KEY_ptr = (volatile unsigned int *)(LW_virtual + KEY_BASE);
    // unsigned int previousButtonState = *KEY_ptr;

    // Set up switch pointer
    volatile unsigned int *SW_ptr = (volatile unsigned int *)(LW_virtual + SW_BASE);
    unsigned int previousSwitchState = *SW_ptr; // Store initial switch state

    // Initialize the LCD canvas and perform drawing operations
    initializeLCDCanvas(&LcdCanvas, virtual_base);
    drawGrid(&LcdCanvas);

    printf("Use switches SW0 to SW3 to input a binary number and display its decimal equivalent on the 7-segment display.\n");

    // init button state to 0 so 1 can be used to end program(key 0)
    unsigned int currentButtonState = 0;

    int table_index = 0;

    // make sure screen is clean
    clearNumbers(&LcdCanvas);

    while (currentButtonState != BUTTON_EXIT) // (key 0)
    {

        //////// Start switches
        currentButtonState = *KEY_ptr;
        unsigned int currentSwitchState = *SW_ptr & 0x0F; // Mask to ensure only the first four switches are read
        if (currentSwitchState != previousSwitchState)
        {
            *JP1_ptr = currentSwitchState; // Set the new state directly to the display
            // add display to SCREEN here
            printf("Displaying number: %u\n", (int)(currentSwitchState));
            usleep(DEBOUNCE_INTERVAL);                // Debounce delay to prevent rapid state changes
            previousSwitchState = currentSwitchState; // Update previous state for comparison
        }
        //////// END switches
        if (currentButtonState == BUTTON_ADD_NUMBER)
        {
            if (table_index < 8)
            {
                storeMatrixValues((int)(currentSwitchState), table_index, matrixA, matrixB);
                printNumberOnLCD(&LcdCanvas, (int)(currentSwitchState), table_index);
                table_index++;
                usleep(DEBOUNCE_INTERVAL); // Sleep to debounce
            }
            else
            {
                // Handle matrix multiplication
                matrix_multiplication(result, matrixA, matrixB);
                print_matrix_multiplication(&LcdCanvas, result);
            }
        }
    }

    clearNumbers(&LcdCanvas);

    // Perform cleanup before exiting the program
    cleanup(virtual_base, fd, &LcdCanvas);
    perform_cleanup();
    return 0;
}

// Initializes hardware for the application by mapping necessary memory regions
int initialize_hardware()
{
    // Attempt to open the memory device file that represents the physical memory of the system
    fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd == -1)
    {
        // Print an error message if the memory device cannot be opened
        perror("ERROR: could not open \"/dev/mem\"");
        return -1;  // Return -1 to indicate failure
    }

    // Map the physical address space for the Lightweight (LW) HPS-to-FPGA bridge into the process's virtual address space
    LW_virtual = mmap(NULL, LW_BRIDGE_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, LW_BRIDGE_BASE);
    if (LW_virtual == MAP_FAILED)
    {
        // Print an error message if memory mapping fails
        perror("ERROR: mmap() failed");
        close(fd);  // Close the file descriptor as cleanup
        return -1;  // Return -1 to indicate failure
    }
    return 0; // Return 0 to indicate success
}

// Maps memory for hardware registers and returns the virtual base address
void *mapMemory(int fd)
{
    // Map a specific memory region specified by HW_REGS_BASE and HW_REGS_SPAN
    void *virtual_base = mmap(NULL, HW_REGS_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, HW_REGS_BASE);
    if (virtual_base == MAP_FAILED)
    {
        // Print an error message if mapping fails
        printf("ERROR: mmap() failed...\n");
        close(fd);  // Close the file descriptor as part of error handling
        return NULL;  // Return NULL to indicate failure
    }
    return virtual_base;  // Return the address of the mapped memory
}


// Initializes the LCD canvas with specified parameters
void initializeLCDCanvas(LCD_CANVAS *canvas, void *virtual_base)
{
    // Display an initial message on console for debug or status update
    printf("Graphic LCD Demo\r\n");

    // Set the width of the LCD display from a predefined constant
    canvas->Width = LCD_WIDTH;

    // Set the height of the LCD display from a predefined constant
    canvas->Height = LCD_HEIGHT;

    // Set the number of bits per pixel, here 1 bit per pixel typically for monochrome
    canvas->BitPerPixel = 1;

    // Calculate the frame size in bytes required for the entire display
    canvas->FrameSize = canvas->Width * canvas->Height / 8; // Dividing by 8 as each pixel is 1 bit

    // Dynamically allocate memory for the frame buffer that holds the display content
    canvas->pFrame = (void *)malloc(canvas->FrameSize);

    // Check if the memory allocation was successful
    if (canvas->pFrame == NULL)
    {
        // If memory allocation failed, print an error message
        printf("failed to allocate lcd frame buffer\r\n");
    }
    else
    {
        // If memory allocation was successful, initialize the LCD hardware
        LCDHW_Init(virtual_base); // Initialize the LCD hardware with the base address of the mapped peripheral

        // Turn on the backlight of the LCD
        LCDHW_BackLight(true);

        // Initialize the LCD display to be ready for use
        LCD_Init();

        // Clear the display, setting it to a white screen
        DRAW_Clear(canvas, LCD_WHITE);
    }
}


// Draws a 4x2 grid on the LCD screen
void drawGrid(LCD_CANVAS *canvas)
{
    int row, col;
    int cellWidth = canvas->Width / 4;   // Dividing by 4 for 4 columns
    int cellHeight = canvas->Height / 2; // Dividing by 2 for 2 rows
    for (row = 0; row < 2; row++)
    { // 2 rows
        for (col = 0; col < 4; col++)
        { // 4 columns
            int x1 = col * cellWidth;
            int y1 = row * cellHeight;
            int x2 = x1 + cellWidth - 1;
            int y2 = y1 + cellHeight - 1;
            DRAW_Rect(canvas, x1, y1, x2, y2, LCD_BLACK); // Draw rectangle in black
        }
    }
}

// Function to perform matrix multiplication for two 2x2 matrices and store the result in a provided array.
void matrix_multiplication(int *result, int *matrixA, int *matrixB)
{
    // Compute the element at the first row, first column of the result matrix.
    result[0] = matrixA[0] * matrixB[0] + matrixA[1] * matrixB[2]; // A[1,1] * B[1,1] + A[1,2] * B[2,1]

    // Compute the element at the first row, second column of the result matrix.
    result[1] = matrixA[0] * matrixB[1] + matrixA[1] * matrixB[3]; // A[1,1] * B[1,2] + A[1,2] * B[2,2]

    // Compute the element at the second row, first column of the result matrix.
    result[2] = matrixA[2] * matrixB[0] + matrixA[3] * matrixB[2]; // A[2,1] * B[1,1] + A[2,2] * B[2,1]

    // Compute the element at the second row, second column of the result matrix.
    result[3] = matrixA[2] * matrixB[1] + matrixA[3] * matrixB[3]; // A[2,1] * B[1,2] + A[2,2] * B[2,2]

    // The resulting 2x2 matrix is stored in 'result' in row-major order:
    // [ result[0] result[1] ]
    // [ result[2] result[3] ]
}


// Function to display the result of a 2x2 matrix multiplication on the LCD
void print_matrix_multiplication(LCD_CANVAS *canvas, int *result)
{
    // Assuming the result matrix is displayed on the entire 4x2 grid
    // Customize the grid position for the result matrix here
    int result_positions[4] = {0, 1, 4, 5}; // Top two cells and bottom two cells of the left half

    // Clear previous numbers
    clearNumbers(canvas); // Clears the entire screen and optionally redraws the grid

    // Loop to print each element of the result matrix
    int i;
    for (i = 0; i < 4; i++)
    {
        int row = result_positions[i] / 4; // Calculating row index
        int col = result_positions[i] % 4; // Calculating column index

        // Calculate position within the cell to center the text
        int cellWidth = canvas->Width / 4;
        int cellHeight = canvas->Height / 2;
        int x = col * cellWidth + (cellWidth - 16) / 2;   // Center text horizontally
        int y = row * cellHeight + (cellHeight - 16) / 2; // Center text vertically

        // Prepare text to be displayed
        char text[10];                  // Ensure the buffer is large enough
        sprintf(text, "%d", result[i]); // Convert integer to string

        // Print each element of the matrix at the calculated position
        DRAW_PrintString(canvas, x, y, text, LCD_BLACK, &font_16x16);
    }

    // Refresh the display to show the updated matrix
    DRAW_Refresh(canvas);
}

void storeMatrixValues(int switches_input, int table_index, int *matrixA, int *matrixB)
{
    // Custom mapping array to order indices as per your specification
    int custom_order[] = {0, 1, 4, 5, 2, 3, 6, 7};

    // Ensure the table_index is within valid range
    if (table_index < 0 || table_index >= 8)
    {
        fprintf(stderr, "Error: Invalid table index %d. Must be between 0 and 7.\n", table_index);
        return;
    }

    // Determine the correct position in the matrix arrays based on the custom mapping
    int mapped_index = custom_order[table_index];

    // Store the input in the appropriate matrix array
    if (mapped_index < 4)
    {
        matrixA[mapped_index] = switches_input; // Store in matrixA for indices 0-3
    }
    else
    {
        matrixB[mapped_index - 4] = switches_input; // Store in matrixB for indices 4-7, adjusting index by subtracting 4
    }
}

// Function to display a number on the LCD based on the user's switch input and a specified order.
void printNumberOnLCD(LCD_CANVAS *canvas, int switches_input, int table_index)
{
    // Define the custom order of indices for displaying numbers in a non-linear matrix format.
    int custom_order[] = {0, 1, 4, 5, 2, 3, 6, 7};

    // Validate that the table_index is within the valid range to prevent accessing out-of-bounds array elements.
    if (table_index < 0 || table_index >= 8)
    {
        return; // Exit the function if the index is out of range to avoid errors.
    }

    // Map the user-defined index to the custom index arrangement for displaying numbers.
    int mapped_index = custom_order[table_index];

    // Determine the row and column based on the mapped index for a 4-column layout.
    int row = mapped_index / 4; // Calculate row from the mapped index (4 columns total).
    int col = mapped_index % 4; // Calculate column from the mapped index (2 rows total).

    // Calculate the width and height for each cell of the display grid.
    int cellWidth = canvas->Width / 4;  // Divide canvas width by the number of columns.
    int cellHeight = canvas->Height / 2; // Divide canvas height by the number of rows.

    // Calculate the coordinates to center the text within each grid cell.
    int x = col * cellWidth + (cellWidth - 16) / 2; // Horizontal position centered within the cell.
    int y = row * cellHeight + (cellHeight - 16) / 2; // Vertical position centered within the cell.

    // Format the switch input as a string to be displayed.
    char text[10];
    sprintf(text, "%d", switches_input); // Convert integer to string for display.

    // Call the graphics library function to draw the string on the canvas at the calculated position.
    DRAW_PrintString(canvas, x, y, text, LCD_BLACK, &font_16x16);

    // Refresh the canvas to update the display with the new data.
    DRAW_Refresh(canvas);
}


void clearNumbers(LCD_CANVAS *canvas)
{
    // Clear the entire screen
    DRAW_Clear(canvas, LCD_WHITE);

    // Optionally, redraw the grid if needed
    drawGrid(canvas);

    // Refresh the LCD to display the changes
    DRAW_Refresh(canvas);
}

// Cleans up by unmapping memory, closing file descriptor, and freeing canvas frame
void cleanup(void *virtual_base, int fd, LCD_CANVAS *canvas)
{
    if (munmap(virtual_base, HW_REGS_SPAN) != 0)
    {
        printf("ERROR: munmap() failed...\n");
    }
    close(fd);

    // Free the canvas frame buffer
    if (canvas->pFrame != NULL)
    {
        free(canvas->pFrame);
        canvas->pFrame = NULL; // Set to NULL to prevent double free
    }
}

// Function to clean up resources to prevent resource leaks.
void perform_cleanup()
{
    // Check if the virtual memory was mapped
    if (LW_virtual)
    {
        // Unmap the memory mapping to free the virtual memory space
        munmap(LW_virtual, LW_BRIDGE_SPAN);
    }
    // Check if the file descriptor is valid
    if (fd != -1)
    {
        // Close the file descriptor to free up system resources
        close(fd);
    }
}
