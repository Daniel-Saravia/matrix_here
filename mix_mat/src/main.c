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
#define BUTTON_ADD_NUMBER 2     // Triggers a specific action, like starting an operation
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
void printNumbers(LCD_CANVAS *canvas);
void cleanup(void *virtual_base, int fd, LCD_CANVAS *canvas);
void clearNumbers(LCD_CANVAS *canvas);
void displayGridOnLCD(LCD_CANVAS *canvas);
void handleInput();
void printNumbers_users(LCD_CANVAS *canvas, int switches_input, int table_index);

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
            { // Check to ensure we do not exceed grid limits
                printNumbers_users(&LcdCanvas, (int)(currentSwitchState), table_index);
                table_index++;
            }
            usleep(DEBOUNCE_INTERVAL); // Sleep to debounce
        }
    }

    clearNumbers(&LcdCanvas);

    // Perform cleanup before exiting the program
    cleanup(virtual_base, fd, &LcdCanvas);
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

// Maps memory for hardware registers and returns the virtual base address
void *mapMemory(int fd)
{
    void *virtual_base = mmap(NULL, HW_REGS_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, HW_REGS_BASE);
    if (virtual_base == MAP_FAILED)
    {
        printf("ERROR: mmap() failed...\n");
        close(fd);
    }
    return virtual_base;
}

// Initializes the LCD canvas with specified parameters
void initializeLCDCanvas(LCD_CANVAS *canvas, void *virtual_base)
{
    printf("Graphic LCD Demo\r\n");
    canvas->Width = LCD_WIDTH;
    canvas->Height = LCD_HEIGHT;
    canvas->BitPerPixel = 1;
    canvas->FrameSize = canvas->Width * canvas->Height / 8;
    canvas->pFrame = (void *)malloc(canvas->FrameSize);
    if (canvas->pFrame == NULL)
    {
        printf("failed to allocate lcd frame buffer\r\n");
    }
    else
    {
        LCDHW_Init(virtual_base);
        LCDHW_BackLight(true);
        LCD_Init();
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

// Prints the number "12" in each cell of a 4x2 grid
void printNumbers(LCD_CANVAS *canvas)
{
    int row, col;
    int cellWidth = canvas->Width / 4;   // 4 columns
    int cellHeight = canvas->Height / 2; // 2 rows

    for (row = 0; row < 2; row++)
    { // 2 rows
        for (col = 0; col < 4; col++)
        { // 4 columns

            int x = col * cellWidth + (cellWidth - 16) / 2;   // Center "123" in the cell
            int y = row * cellHeight + (cellHeight - 16) / 2; // Center "123" in the cell

            DRAW_PrintString(canvas, x, y, "12", LCD_BLACK, &font_16x16);
        }
    }

    DRAW_Refresh(canvas); // Refresh the canvas to update the display
}

// Prints user input numbers in each cell of a 4x2 grid
void printNumbers_users(LCD_CANVAS *canvas, int switches_input, int table_index) {
    // Custom mapping array to order indices as per your specification
    int custom_order[] = {0, 1, 4, 5, 2, 3, 6, 7};

    // Check if the table_index is valid
    if (table_index < 0 || table_index >= 8) {
        fprintf(stderr, "Error: Invalid table index %d. Must be between 0 and 7.\n", table_index);
        return;
    }

    // Map the table_index to our custom grid order
    int mapped_index = custom_order[table_index];

    // Convert linear index to 2D grid positions
    int row = mapped_index / 4;  // 4 columns in total
    int col = mapped_index % 4;  // 2 rows in total

    // Calculate cell dimensions
    int cellWidth = canvas->Width / 4;
    int cellHeight = canvas->Height / 2;

    // Calculate text position within the cell
    int x = col * cellWidth + (cellWidth - 16) / 2;
    int y = row * cellHeight + (cellHeight - 16) / 2;

    // Convert switches_input to string
    char text[10];
    sprintf(text, "%d", switches_input);

    // Draw the text in the corresponding cell
    DRAW_PrintString(canvas, x, y, text, LCD_BLACK, &font_16x16);

    // Refresh the display to show changes
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
