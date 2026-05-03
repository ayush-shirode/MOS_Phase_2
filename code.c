#include<stdio.h>
#include <stdbool.h>
#include<time.h>
#include "utils.h"
#include "globals.h"

FILE *fin, *fout;

void startExecution();
void executeProgram();
void MOS();
void read();
void write();
void terminate();

int VA;
int PI = 0;
int faultVA;   // store faulting VA
int TI = 0;

int addressMap(int VA) {
    int page = VA / 10;
    int offset = VA % 10;

    if (pageTable[page] == -1) {
        PI = 3;
        faultVA = VA;
        return -1;
    }

    return pageTable[page] * 10 + offset;
}

int getRA(int VA) {
    int RA = addressMap(VA);
    return RA;
}

// INIT Function (initialize the system by empty variable)
void init() {
    for (int i = 0; i < 300; i++)
        for (int j = 0; j < 4; j++)
            M[i][j] = ' ';

    for (int i = 0; i < 4; i++) {
        IR[i] = ' ';
        R[i] = ' ';
    }

    IC = 0;
    C = 0;
    SI = 0;

    // Assign Page table register using random number generator
    for (int i = 0; i < 10; i++) {
        pageTable[i] = -1;
    }

    for (int i = 0; i < 30; i++) {
        frameUsed[i] = false;
    }

    int frame = randomNumberGenerator();
    PTR = frame * 10;
    
    VA = 0;             // Stores virtual address
}

void load() {
    char word[1024];     // Stores each instruction which is to be executed

    while (fgets(word, sizeof(word), fin)) {
        // printf("%s", word);

        if (cmpString(word, "$AMJ", 4)) {
            printf("%s", "AMJ (initializing the String)\n");

            TL = 0;
            LL = 0;
            // Time Limit calculation
            countLimit(word, &TL, 8, 12);

            // Line Limit calculation
            countLimit(word, &LL, 12, 16);

            init();
        }
        else if (cmpString(word, "$DTA", 4)) {
            printf("%s", "DTA (Starting Execution)\n");
            startExecution();
        }

        else if (cmpString(word, "$END", 4)) {
            printf("%s", "END (Instruction Completed)\n");
            continue;
        }
        else {
            int k = 0;
            while (k < lenString(word) && word[k] != '\n')
            {

                // Validation for memory overflow
                if (VA >= 300) {
                    printf("Memory overflow\n");
                    return;
                }                

                int page = VA / 10;
                int offset = VA % 10;
                if (pageTable[page] == -1) {
                    int frame = randomNumberGenerator();
                    pageTable[page] = frame;
                }
                
                int RA = pageTable[page] * 10 + offset;

                for (int j = 0; j < 4; j++) {
                    if (word[k] != '\n' && word[k] != '\0')
                        M[RA][j] = word[k++];
                    else
                        M[RA][j] = ' ';
                }

                VA++;
            }
        }
    }
}

// START EXECUTION
void startExecution() {
    IC = 0;
    executeProgram();
}

void executeProgram() {
    int step = 0;
    while (true)
    {
        // To avoid Infinite loop
        if (step++ > 1000) {
            printf("Infinite loop detected\n");
            break;
        }

        // Fetch the Instruction
        int RA = addressMap(IC);
        if (PI != 0) {
            MOS(); 
            continue;
        }

        if (RA == -1) {
            MOS();
            continue;
        }

        // Fetch instruction from REAL ADDRESS
        for (int i = 0; i < 4; i++) {
            IR[i] = M[RA][i];
        }

        IC++;
        
        // Decode and Execute the Instructions

        // Handle H
        if (IR[0] == 'H' && IR[1] == ' ' && IR[2] == ' ' && IR[3] == ' ') {
            TL--;
            if (TL < 0) { 
                TI = 2; 
                MOS(); 
                break; 
            }
            SI = 3;
            MOS();
            break;
        }

        // Validating Instructions
        bool found_first = false;
        bool found_second = false;
        for (int i = 0; i < 7; i++) {
            if (IR[0] == valid_first[i]) found_first = true;
        }
        for (int i = 0; i < 3; i++) {
            if (IR[1] == valid_second[i]) found_second = true;
        }

        if (!found_first || !found_second) {
            printf("%s", "Invalid Instruction !!\n");
            return;
        }

        // LR Instruction (load Register)
        if (IR[0] == 'L' && IR[1] == 'R') {
            int addr = (IR[2]-'0')*10 + (IR[3]-'0');
            if (!(IR[2] >= '0' && IR[2] <= '9' && IR[3] >= '0' && IR[3] <= '9')) {
                printf("Invalid operand\n");
                break;
            }
            if (addr < 0 || addr >= 300) {
                printf("Memory address out of bounds\n");
                break;
            }
            TL--;
            if (TL < 0) {
                TI = 2;
                MOS();
                break;
            }

            int RA = getRA(addr);
            for (int i = 0; i < 4; i++) R[i] = M[RA][i];
        }

        // SR (Store Register)
        else if (IR[0] == 'S' && IR[1] == 'R') {
            int addr = (IR[2]-'0')*10 + (IR[3]-'0');
            if (!(IR[2] >= '0' && IR[2] <= '9' && IR[3] >= '0' && IR[3] <= '9')) {
                printf("Invalid operand\n");
                break;
            }
            if (addr < 0 || addr >= 300) {
                printf("Memory address out of bounds\n");
                break;
            }
            TL--;
            if (TL < 0) {
                TI = 2;
                MOS();
                break;
            }

            int RA = getRA(addr);
            for (int i = 0; i < 4; i++) M[RA][i] = R[i];
        }
        
        // CR (Compare Register)
        else if (IR[0] == 'C' && IR[1] == 'R') {
            int addr = (IR[2]-'0')*10 + (IR[3]-'0');
            if (!(IR[2] >= '0' && IR[2] <= '9' && IR[3] >= '0' && IR[3] <= '9')) {
                printf("Invalid operand\n");
                break;
            }
            if (addr < 0 || addr >= 300) {
                printf("Memory address out of bounds\n");
                break;
            }
            C = 1;
            int RA = getRA(addr);
            for (int i = 0; i < 4; i++) {
                if (R[i] != M[RA][i]) {
                    C = 0;
                    break;
                }
            }
            TL--;
            if (TL < 0) {
                TI = 2;
                MOS();
                break;
            }

        }

        // BT (Branch if True)
        else if (IR[0] == 'B' && IR[1] == 'T') {
            int addr = (IR[2]-'0')*10 + (IR[3]-'0');
            if (!(IR[2] >= '0' && IR[2] <= '9' && IR[3] >= '0' && IR[3] <= '9')) {
                printf("Invalid operand\n");
                break;
            }
            if (addr < 0 || addr >= 300) {
                printf("Memory address out of bounds\n");
                break;
            }
            TL--;
            if (TL < 0) {
                TI = 2;
                MOS();
                break;
            }

            if (C == 1) IC = addr;
        }
        
        // GD (Get Data)
        else if (IR[0] == 'G' && IR[1] == 'D') {
            SI = 1;
            TL--;
            if (TL < 0) {
                TI = 2;
                MOS();
                break;
            }

            MOS();
        }

        // PD (Put Data)
        else if (IR[0] == 'P' && IR[1] == 'D') {
            SI = 2;
            TL--;
            if (TL < 0) {
                TI = 2;
                MOS();
                break;
            }

            MOS();
        }
        else {
            printf("Invalid instruction: %c%c%c%c\n", IR[0], IR[1], IR[2], IR[3]);
            break;
        }        
    }
}

void read() {
    char word[40];
    int addr = (IR[2]-'0')*10 + (IR[3]-'0');

    // reading if Input file is not Empty
    if (fgets(word, sizeof(word), fin) == NULL) {
        printf("No data available for GD\n");
        return;
    }

    int k = 0;
    int VA = addr;

    for (int i = 0; i < 10; i++) {
        int RA = getRA(addr + i);
        if (PI != 0) { 
            MOS(); 
            return; 
        }
        for (int j = 0; j < 4; j++) {
            if (word[k] != '\0' && word[k] != '\n')
                M[RA][j] = word[k++];
            else
                M[RA][j] = ' ';
        }
    }
}

void write() {
    int addr = (IR[2]-'0')*10 + (IR[3]-'0');

    // Implememted Line Limit Exceeded Logic
    if (LL <= 0) {
        printf("%s", "Line Limit Exceeded !!\n");
        return;
    }

    for (int i = 0; i < 10; i++) {
        int RA = getRA(addr + i);
        if (PI != 0) { 
            MOS(); 
            return; 
        }
        for (int j = 0; j < 4; j++)
            fputc(M[RA][j], fout);
    }

    fputc('\n', fout);
    LL--;
}

void terminate() {
    fprintf(fout, "\n\n");
}

void handlePageFault() {
    int page = faultVA / 10;
 
    int frame = randomNumberGenerator();
    pageTable[page] = frame;
 
    PI = 0;
    IC--;
}


void MOS() {
    if (TI == 2) {
        printf("Time Limit Exceeded\n");
        terminate();
        TI = 0;
        return;
    }

    if (PI == 3) {
        handlePageFault();
        return;
    }

    if (SI == 1) {
        read();
        SI = 0;
    }
    else if (SI == 2) {
        write();
        SI = 0;
    }
    else if (SI == 3) {
        terminate();
        SI = 0;
    }
}

int main() {
    clock_t start, end;

    start = clock();

    fin = fopen("input.txt", "r");
    fout = fopen("output.txt", "w");

    if (fin == NULL) {
        printf("Input file not found\n");
        return 0;
    }
    if (fout == NULL) {
        printf("Output file error\n");
        return 0;
    }

    load();

    fclose(fin);
    fclose(fout);

    end = clock(); 
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Execution time: %f seconds\n", time_taken);
    
    return 0;
}


/*
// Validations Added for instruction as well as operand
1. Checking if Input and Output File exists
2. checking Memery Overflow
3. Invalid Instruction
4. Avoid Infinite Loop if Halt Instruction is not available
5. Added Validation for Correct Operand
6. Vadidation if input file is Empty
*/