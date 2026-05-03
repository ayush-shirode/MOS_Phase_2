#include "globals.h"

bool frameUsed[30];
// Declaring the required Memory and Registers
char M[300][4];     // main storage of size 100 x 4
char IR[4];         // IR (Instruction register) of size 4
char R[4];          // R (General Purpose register) of size 4
int IC;             // Instruction Counter
int C;              // Toggle Register
int SI;             // Supervisor Interept
int TL;
int LL;
char valid_first[] = {76, 83, 67, 66, 71, 80, 72};
char valid_second[] = {82, 84, 68};
int PTR;
int pageTable[10];