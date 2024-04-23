# Matrix Multiplier System README
![Uploading image.png…]()

## Overview
This project is designed to demonstrate matrix multiplication using the DE10-Standard FPGA board. It utilizes a combination of hardware and software components to input and display two 2x2 matrices, perform multiplication, and display the resulting matrix. This system is especially suitable for educational purposes where understanding matrix operations is desired.

## Technologies and Tools
- **DE10-Standard Board**: Utilizes its FPGA capabilities for processing matrix operations and managing user input/output.
- **LED and 7-Segment Display**: Provides visual feedback on system status and matrix values.
- **LCD Display**: Used to display the matrices and results, facilitating user interaction and enhancing the visual output of matrix calculations.
- **Buttons**: Enable user interaction, allowing them to input matrix values and initiate matrix multiplication.
- **C Programming Language**: Used for developing the embedded software that handles matrix input, processing, and display logic.
- **Verilog**: Employed for implementing low-level digital logic on the FPGA for handling displays and inputs.
- **Quartus Prime**: Used for compiling Verilog code and configuring the FPGA with tools for pin planning, simulation, and programming the device.
- **Visual Studio Code**: Acts as the Integrated Development Environment (IDE) for writing and managing C code.
- **FileZilla**: Facilitates file transfer from a development machine (such as a laptop) to the DE10-Standard board.
- **Putty**: Provides a terminal interface to interact with the board, execute programs, and manage system operations.
- **SoC EDS Command Shell**: Used for cross-compiling C code, ensuring that it is compatible with the ARM processor on the DE10-Standard board.

## System Setup
### Hardware Setup
- Connect the DE10-Standard board to a power source and to your computer via USB.
- Ensure that all necessary drivers and interfaces are correctly configured.

### Software Installation
- Install Quartus Prime for FPGA programming.
- Set up Visual Studio Code with necessary extensions for C programming.
- Install PuTTY for SSH and terminal access to the FPGA board.
- Install FileZilla for transferring files to and from the FPGA board.

### Development and Compilation
- Use Quartus Prime to compile the Verilog code and program the FPGA.
- Use the SoC EDS Command Shell to cross-compile the C code developed in Visual Studio Code.

### Execution
- Transfer the compiled binary to the FPGA board using FileZilla.
- Use PuTTY to execute the binary on the board.

## Usage
### Inputting Data
- Use the buttons on the DE10-Standard board to input values for the two matrices. Each button press corresponds to a matrix entry.
- The current input value is displayed on the 7-segment display, and the matrix is visualized on the LCD screen.

### Performing Matrix Multiplication
- Once all entries are inputted, press the designated button (BUTTON_ADD_NUMBER) to compute the matrix multiplication.
- The result is then displayed on the LCD screen, allowing for immediate verification and study.

## Maintenance and Support
- For technical support or maintenance assistance, contact the system administrator or refer to the DE10-Standard board's technical documentation.
- Firmware and software updates will be periodically released to enhance functionality and address any identified issues.

## Conclusion
The Matrix Multiplier System is a robust educational tool that leverages the powerful features of the DE10-Standard FPGA board. It demonstrates how hardware and software can be integrated to perform complex mathematical operations and provides a practical learning platform for students and enthusiasts interested in digital design and embedded systems.

