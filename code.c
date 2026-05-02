// cd "/home/ayush/OS_Assignment/CP_Phase2/" && gcc code.c utils.c -o code && "/home/ayush/OS_Assignment/CP_Phase2/"code

#include<stdio.h>
#include <stdbool.h>
#include<time.h>
#include "utils.h"

// Declaring the required Memory and Registers
char M[100][4];     // main storage of size 100 x 4
char IR[4];         // IR (Instruction register) of size 4
char R[4];          // R (General Purpose register) of size 4
int IC;             // Instruction Counter
int C;              // Toggle Register
int SI;             // Supervisor Interept
int TL;
int LL;
char valid_first[] = {76, 83, 67, 66, 71, 80, 72};
char valid_second[] = {82, 84, 68};


FILE *fin, *fout;

void startExecution();
void executeProgram();
void MOS();
void read();
void write();
void terminate();

// INIT Function (initialize the system by empty variable)
void init() {
    for (int i = 0; i < 100; i++)
        for (int j = 0; j < 4; j++)
            M[i][j] = ' ';

    for (int i = 0; i < 4; i++) {
        IR[i] = ' ';
        R[i] = ' ';
    }

    IC = 0;
    C = 0;
    SI = 0;
}

void load() {
    int m = 0;          // It Points location at which instructions are stored

    char word[100];     // Stores each instruction which is to be executed

    while (fgets(word, sizeof(word), fin)) {
        // printf("%s", word);

        if (cmpString(word, "$AMJ", 4)) {
            printf("%s", "AMJ (initializing the String)\n");

            // Time Limit calculation
            countLimit(word, &TL, 8, 12);

            // Line Limit calculation
            countLimit(word, &LL, 12, 16);

            init();
            m = 0;
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
                if (m >= 100) {
                    printf("Memory overflow\n");
                    return;
                }                
                for (int j = 0; j < 4 && word[k] != '\n' && word[k] != '\0'; j++) {
                    M[m][j] = word[k++];
                }
                m++;
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
        if (TL <= 0) {
            printf("%s", "Time limit Exceeded !!\n");
            break;
        }

        // To avoid Infinite loop
        if (step++ > 1000) {
            printf("Infinite loop detected\n");
            break;
        }

        // Fetch the Instruction
        for (int i = 0; i < 4; i++) {
            IR[i] = M[IC][i];
        }

        IC++;
        
        // Decode and Execute the Instructions


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
            if (addr < 0 || addr >= 100) {
                printf("Memory address out of bounds\n");
                break;
            }
            TL--;
            for (int i = 0; i < 4; i++) R[i] = M[addr][i];
        }

        // SR (Store Register)
        else if (IR[0] == 'S' && IR[1] == 'R') {
            int addr = (IR[2]-'0')*10 + (IR[3]-'0');
            if (!(IR[2] >= '0' && IR[2] <= '9' && IR[3] >= '0' && IR[3] <= '9')) {
                printf("Invalid operand\n");
                break;
            }
            if (addr < 0 || addr >= 100) {
                printf("Memory address out of bounds\n");
                break;
            }
            TL--;
            for (int i = 0; i < 4; i++) M[addr][i] = R[i];
        }
        
        // CR (Compare Register)
        else if (IR[0] == 'C' && IR[1] == 'R') {
            int addr = (IR[2]-'0')*10 + (IR[3]-'0');
            if (!(IR[2] >= '0' && IR[2] <= '9' && IR[3] >= '0' && IR[3] <= '9')) {
                printf("Invalid operand\n");
                break;
            }
            if (addr < 0 || addr >= 100) {
                printf("Memory address out of bounds\n");
                break;
            }
            C = 1;
            for (int i = 0; i < 4; i++) {
                if (R[i] != M[addr][i]) {
                    C = 0;
                    break;
                }
            }
            TL--;
        }

        // BT (Branch if True)
        else if (IR[0] == 'B' && IR[1] == 'T') {
            int addr = (IR[2]-'0')*10 + (IR[3]-'0');
            if (!(IR[2] >= '0' && IR[2] <= '9' && IR[3] >= '0' && IR[3] <= '9')) {
                printf("Invalid operand\n");
                break;
            }
            if (addr < 0 || addr >= 100) {
                printf("Memory address out of bounds\n");
                break;
            }
            TL--;
            if (C == 1) IC = addr;
        }
        
        // GD (Get Data)
        else if (IR[0] == 'G' && IR[1] == 'D') {
            SI = 1;
            TL--;
            MOS();
        }

        // PD (Put Data)
        else if (IR[0] == 'P' && IR[1] == 'D') {
            SI = 2;
            TL--;
            MOS();
        }

        // H (Halt)
        else if (IR[0] == 'H') {
            SI = 3;
            TL--;
            MOS();
            break;
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
    for (int i = addr; i < addr + 10 && i < 100; i++) {
        for (int j = 0; j < 4; j++) {
            if (word[k] == '\n' || word[k] == '\0')
                M[i][j] = ' ';
            else
                M[i][j] = word[k++];
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

    for (int i = addr; i < addr + 10 && i < 100; i++) {


        for (int j = 0; j < 4; j++) {
            fputc(M[i][j], fout);
        }
    }
    fputc('\n', fout);
    LL--;
}

void terminate() {
    fprintf(fout, "\n\n");
}

void MOS() {
    if (SI == 1)
        read();
    else if (SI == 2)
        write();
    else if (SI == 3)
        terminate();
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
// Validations Added
1. Checking if Input and Output File exists
2. checking Memery Overflow
3. Invalid Instruction
4. Avoid Infinite Loop if Halt Instruction is not available
5. Added Validation for Correct Operand
6. Vadidation if input file is Empty
*/