#ifndef GLOBALS_H
#define GLOBALS_H
#include<stdbool.h>

extern bool frameUsed[30];
// Declaring the required Memory and Registers

extern char M[300][4];     // main storage of size 100 x 4
extern char IR[4];         // IR (Instruction register) of size 4
extern char R[4];          // R (General Purpose register) of size 4
extern int IC;             // Instruction Counter
extern int C;              // Toggle Register
extern int SI;             // Supervisor Interept
extern int TL;
extern int LL;
extern char valid_first[7];
extern char valid_second[3];
extern int PTR;
extern int pageTable[10];

#endif