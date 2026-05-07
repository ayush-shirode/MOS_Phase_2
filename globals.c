#include "globals.h"

bool frameUsed[30];
// Declaring the required Memory and Registers
char M[300][4];     // main storage of size 300 x 4
char IR[4];         // IR (Instruction register) of size 4
char R[4];          // R (General Purpose register) of size 4
int IC;             // Instruction Counter
int C;              // Toggle Register
int SI;             // Supervisor Interrupt
int TL;             // Time Limit
int LL;             // Line Limit
int TTC;            // Total Time Counter
int LLC;            // Line Limit Counter
// L=76, S=83, C=67, B=66, G=71, P=80, H=72
char valid_first[] = {76, 83, 67, 66, 71, 80, 72};
// R=82, T=84, D=68, R=82
char valid_second[] = {82, 84, 68, 82};
int PTR;
int pageTable[10];
char jobID[5];      // Job ID (4 chars + null)
int EM;             // Error flag: 0=no error, 1=TLE, 2=LLC exceeded, 3=operand error