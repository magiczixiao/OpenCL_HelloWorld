Generate a .aocx to run on x86 emulator:

aoc device/HelloWorld_Kernel.cl -march=emulator

Generate run.exe:
g++ host/main.cpp -o main.exe -I include/

