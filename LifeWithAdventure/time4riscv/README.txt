Project name: Life With Adventure
Authors: Frans Ekberg and Chandadeep Kaur
Course: IS1200
Date: 10 October 2025

== Description ==

This program is a text-based adventure game written in C and RISC-V assembly. The player can explore rooms, talk to NPCs, collect items and fight enemies.

== How to compile ==

    1. Open a terminal and navigate to the directory "time4riscv" where the make-file and the project files are located.
    2. Run the following command to assemble the code: 
       make

== How to run ==

    1. Run "jtagd --user-start" in the terminal.
    2. Reset the RISC-V board using "KEY0".
    3. Run "dtekv-run main.bin" in the terminal.
    4. The peripherals on the board are used to make choices in the game:
       - Switches 1-5 are used to select different options and progress through the game, where option "1." is chosen by toggling switch 1 and so on.
       - Button 1 (KEY1) is used to perform various actions, such as the jump sequence in room 8 (Abandoned Bridge).
       - Switch 0 is used to view the inventory.

