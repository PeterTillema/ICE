# ICE Compiler [![Build Status](https://api.travis-ci.org/PeterTillema/ICE.svg)](https://travis-ci.org/PeterTillema/ICE) [![IRC badge](https://img.shields.io/badge/IRC%20channel-%23icedev%20on%20EFNet-blue.svg)](http://chat.efnet.org/irc.cgi?adv=1&nick=ICEuser&chan=%23icedev)
![](http://i.imgur.com/yLPnSG7.png)

ICE Compiler is a program that compiles TI-BASIC-like language to assembly. Create insane games, take full advanage of the color screen, and close the gap to using other languages! Not only will your BASIC programs enjoy increased speed, it is also easy to use. Type your program in the normal BASIC editor, compile within seconds\*, for hours of fun! No shell is required.

*\*Note: compilation time depends on the size of your source program(s)*

## Build
To build ICE for the calculator, be sure to have the [C SDK](https://github.com/CE-Programming/toolchain/releases) installed. Then, run `make` from the command line. To build ICE for the computer, run `make -f makefile.computer` from the command line.

## Download
See the [Cemetech Archives](https://www.cemetech.net/programs/index.php?mode=file&path=/84pce/asm/programs/ICECompiler.zip) for the latest download and documentation.

## Usage
### Calculator
Run `Asm(prgmICE` from the homescreen. This will show a list of compilable programs. Use the `[UP]` and `[DOWN]` keys to select the right program, and press `[ENTER]` or `[2ND]`. Your program will now be compiled!  You can now run your program with `Asm(prgmNAME` or press `[Y=]` after compiling.

*If you have OS 5.3+, you can omit the `Asm(` token and simply run `prgmICE` from the homescreen*

### Computer (CLI)
From the command line, run `ice <input program>` to compile the input programs. Note that if you want to compile subprograms too, put them in the same folder as `ice.exe`. It is a good idea to put `ice.exe` and all of your input programs in the same folder.

## Bugs
If you find a bug, or if you see the message `ICE ERROR: please report it!`, please make an issue here or post it in the relevant [Cemetech topic](https://www.cemetech.net/forum/viewtopic.php?t=12616).

*Note: Please provide your code, it significantly helps with debugging.*
