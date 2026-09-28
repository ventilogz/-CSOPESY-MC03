CSOPESY Machine Output 3: Marquee Console
==========================================================

Group 7 Members
---------------
- Lance Krystofer Galicia
- Raina Helaga
- Maurienne Marie Mojica
- Venice Raeka Plurad

Entry File
----------
console_ui.cpp (contains the main() function)

Other source files:
- CommandInterpreter.cpp / CommandInterpreter.h
- MarqueeEngine.cpp / MarqueeEngine.h
- Terminal.cpp / Terminal.h

GitHub Repository (Alternative)
--------------------------------
If you'd rather view or clone the source directly, the project is
also available at:
https://github.com/ventilogz/-CSOPESY-MC03


About the Program
-----------------
This is a console-based OS emulator with a command interpreter and an
animated ASCII marquee. A jeepney drives across a Manila skyline at the
top of the screen while its roof signboard scrolls the marquee text.
Commands are typed below the scene, and the animation keeps running
while you type.


Requirements (Windows)
----------------------
- Windows 10 or 11
- g++ and gdb from MinGW-w64 (for example, installed through MSYS2)
  Both "g++ --version" and "gdb --version" should work in PowerShell.
- Visual Studio Code with the "C/C++" extension by Microsoft
- A console window at least 80 columns wide and 21 rows tall
  (maximizing the window is recommended)


How to Run (Visual Studio Code)
-------------------------------
1. Open the project folder in Visual Studio Code.
2. Open the Run and Debug panel (Ctrl+Shift+D).
3. Select "run mc03 (Windows)" from the dropdown at the top.
4. Press F5 or click the green Run button.
5. The program builds automatically and opens in a new console window.
   Maximize the window before typing commands.


How to Run (Command Line)
-------------------------
Open PowerShell or Command Prompt in the project folder, then run:

    g++ -std=c++17 -pthread console_ui.cpp CommandInterpreter.cpp MarqueeEngine.cpp Terminal.cpp -o mc03
    mc03


Commands
--------
help                  Displays the commands and their descriptions
start_marquee         Starts the marquee animation
stop_marquee          Stops the marquee animation
set_text <text>       Sets the text shown on the jeepney's signboard
                      (1 to 1000 characters)
set_speed <ms>        Sets the marquee refresh interval in milliseconds
                      (whole number from 1 to 60000 ms; decimals, negative
                      numbers and 0 are rejected)
exit                  Terminates the console

Notes:
- Commands are case-sensitive (use "help", not "HELP").
- The default text is "TAFT - VITO CRUZ" and the default speed is 200 ms.
- Up and Down arrow keys recall previously typed commands.
- Ctrl+C also exits the program cleanly.


Screen Layout
-------------
Rows 1 to 11     Jeepney marquee scene (fixed)
Row 12           Divider
Row 13 onward    Header, command prompt, and output (scrolls)

Avoid resizing the window while the program is running, since the
layout is set up once at startup.