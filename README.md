# Processor-Assignment1
This is a simple MIPS simulator in C, class project for learning C and assembly language/computer systems. Will continue to
add more features such as better memory management and less global dependancy.

Most all of the code is in main.c
To run once in directory:
$ gcc -o main main.c -pthreads
$ ./main

It will currently run a bubble sort MIPs file, outputting a sorted list of numbers in memory. Other files include fibonacci.asm,
gcd.asm, function.asm, and other singular test files found inside.To change, alter text file in main().

Supports a multithreaded pipeline, as well as having a direct mapped cache for memory efficency (tracks hits and misses).

Very simple hazard checking, so it will only run concurrently on more simple instructions/less dependant instructions.


