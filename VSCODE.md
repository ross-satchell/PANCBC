Building and Programming the ATtiny1627 in VS Code
=====================================================

This project uses the MPLAB X VS Code extension to build, and a VS Code Task
to program the device via the Curiosity Nano board.

Follow these steps in order. If this is your first time, start at Prerequisites.


Prerequisites (one-time setup)
--------------------------------

The following must be installed before you begin:

1. VS Code
   Download and install from: https://code.visualstudio.com

2. MPLAB X Extension for VS Code
   - Open VS Code
   - Click the square Extensions icon on the left sidebar (or press Ctrl+Shift+X)
   - Search for:  MPLAB X
   - Install the extension published by Microchip Technology
   - Restart VS Code when prompted

3. XC8 Compiler v3.10
   Download and install from:
   https://www.microchip.com/en-us/tools-resources/develop/mplab-xc-compilers
   - Choose the Linux installer
   - Run the installer and accept all default paths

4. pymcuprog (programs the chip over USB)
   - Open a terminal (press Ctrl+` inside VS Code, or open a system terminal)
   - Run this command:
       pip install pymcuprog
   - Wait for it to finish, then close the terminal


Opening the Project in VS Code
--------------------------------

1. Open VS Code
2. Click  File > Open Folder
3. Navigate to and select the  MPLABXProjects  folder, then click Open
4. If VS Code asks "Do you trust the authors of the files in this folder?"
   click  Yes, I trust the authors


Step 1 - Build the Project
----------------------------

Building compiles the C source code into a program file the chip can run.
You must do this before programming.

1. Save any open source files with Ctrl+S
2. Press  Ctrl+Shift+P  to open the Command Palette
   (a search bar will appear at the top of the screen)
3. Type:  Tasks: Run Build Task  and press Enter
   (you can also just press  Ctrl+Shift+B  as a shortcut)
4. Watch the Terminal panel that opens at the bottom of the screen

A successful build ends with:
    BUILD SUCCESSFUL

If it shows  BUILD FAILED  there is an error in the source code that must be
fixed before you can program the device. The terminal output will show which
file and line number has the problem.

After a successful build, the compiled program is saved here:
    out/Tiny2_BM_Deep_Sleep_CapTouch_Wake/default.elf


Step 2 - Program the Device
------------------------------

This sends the compiled program to the ATtiny1627 chip over USB.

Before you start:
  - Plug the Curiosity Nano board into your PC with a USB cable
  - The green power LED on the board should turn on

1. Press  Ctrl+Shift+P  to open the Command Palette
2. Type:  Tasks: Run Task  and press Enter
3. A list appears - select  Program ATtiny1627  and press Enter
4. If asked "Scan the task output?" choose  Continue without scanning
5. Watch the Terminal panel at the bottom of the screen

A successful programming run looks like this:
    Connecting to nedbg tool...
    Programming ATtiny1627...
    Verifying...
    Programming successful

The device is now running the new program.


Troubleshooting
-----------------

"pymcuprog: command not found"
    Open a terminal and run:  pip install pymcuprog
    Then try Step 2 again.

"No tool connected" or "Could not connect to tool"
    - Check the USB cable is firmly plugged in at both ends
    - Try a different USB port on your PC
    - Unplug, wait 5 seconds, plug back in, then try again

"BUILD FAILED"
    There is a problem in the source code. Read the error lines in the
    Terminal - they will state the file name and line number to look at.
    Fix the error, save the file, and repeat Step 1.

The build succeeded but the old program is still running on the chip
    Make sure Step 2 completed with "Programming successful" at the end.
    If the task seemed to skip or close instantly, check that the Curiosity
    Nano board is connected before running the task.
