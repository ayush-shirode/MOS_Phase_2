#ifndef GLOBALS_H
#define GLOBALS_H
#include<stdbool.h>

extern bool frameUsed[30];

extern char M[300][4];     // main storage of size 300 x 4
extern char IR[4];         // IR (Instruction register) of size 4
extern char R[4];          // R (General Purpose register) of size 4
extern int IC;             // Instruction Counter
extern int C;              // Toggle Register
extern int SI;             // Supervisor Interrupt
extern int TL;             // Time Limit
extern int LL;             // Line Limit
extern int TTC;            // Total Time Counter
extern int LLC;            // Line Limit Counter
extern char valid_first[7];
extern char valid_second[4];
extern int PTR;
extern int pageTable[10];
extern char jobID[5];      // Job ID (4 chars + null)
extern int EM;             // Error flag for termination messages

#endif