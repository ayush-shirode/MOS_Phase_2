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
void terminate(int errorCode);
void handlePageFault();
void printMemory();

int VA;
int PI = 0;
int faultVA;
int TI = 0;

int addressMap(int VA) {
    int page = VA / 10;
    int offset = VA % 10;

    if (page < 0 || page >= 10) {
        PI = 3;
        faultVA = VA;
        return -1;
    }

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
    PI = 0;
    TI = 0;
    TTC = 0;
    LLC = 0;
    EM = 0;

    for (int i = 0; i < 10; i++) {
        pageTable[i] = -1;
    }

    for (int i = 0; i < 30; i++) {
        frameUsed[i] = false;
    }

    int frame = randomNumberGenerator();
    PTR = frame * 10;
    printf("Page Table Location: %d", PTR);
    
    VA = 0;
}

void printMemory() {
    printf("\n========== MEMORY DUMP ==========\n");
    for (int i = 0; i < 300; i++) {
        bool empty = true;
        for (int j = 0; j < 4; j++) {
            if (M[i][j] != ' ') {
                empty = false;
                break;
            }
        }
        if (!empty) {
            printf("M[%3d]: ", i);
            for (int j = 0; j < 4; j++) {
                if (M[i][j] >= 32 && M[i][j] <= 126)
                    printf("%c", M[i][j]);
                else
                    printf(".");
            }
            printf("\n");
        }
    }
    printf("=================================\n\n");
}

void load() {
    char word[1024];

    while (fgets(word, sizeof(word), fin)) {

        if (cmpString(word, "$AMJ", 4)) {
            printf("%s", "AMJ (initializing the Job)\n");

            jobID[0] = word[4];
            jobID[1] = word[5];
            jobID[2] = word[6];
            jobID[3] = word[7];
            jobID[4] = '\0';

            TL = 0;
            LL = 0;

            countLimit(word, &TL, 8, 12);

            countLimit(word, &LL, 12, 16);

            printf("Job ID: %s, Time Limit: %d, Line Limit: %d\n", jobID, TL, LL);

            init();
            continue;
        }
        else if (cmpString(word, "$DTA", 4)) {
            printf("%s", "DTA (Starting Execution)\n");

            printf("\n--- Memory state BEFORE execution ---\n");
            printMemory();

            startExecution();

            printf("\n--- Memory state AFTER execution ---\n");
            printMemory();

            while (fgets(word, sizeof(word), fin)) {
                if (cmpString(word, "$END", 4)) {
                    printf("%s", "END (Job Completed)\n\n");
                    break; 
                }
            }
            continue;
        }

        int k = 0;
        while (k < lenString(word) && word[k] != '\n' && word[k] != '\r')
        {
            if (VA >= 100) {
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

            printf("%d ", RA);

            for (int j = 0; j < 4; j++) {
                if (word[k] != '\n' && word[k] != '\0' && word[k] != '\r')
                    M[RA][j] = word[k++];
                else
                    M[RA][j] = ' ';
            }

            VA++;
        }
    }
}

void startExecution() {
    IC = 0;
    executeProgram();
}

void executeProgram() {
    int step = 0;
    while (true)
    {
        if (step++ > 1000) {
            printf("Infinite loop detected\n");
            break;
        }

        PI = 0;

        int RA = addressMap(IC);
        if (PI != 0) {
            handlePageFault();
            PI = 0;
            RA = addressMap(IC);
            if (PI != 0) {
                printf("Fatal: Cannot resolve page fault for IC=%d\n", IC);
                terminate(0);
                return;
            }
        }

        for (int i = 0; i < 4; i++) {
            IR[i] = M[RA][i];
        }

        IC++;

        if (IR[0] == 'H' && IR[1] == ' ' && IR[2] == ' ' && IR[3] == ' ') {
            SI = 3;
            MOS();
            break;
        }

        bool found_first = false;
        bool found_second = false;
        for (int i = 0; i < 7; i++) {
            if (IR[0] == valid_first[i]) found_first = true;
        }
        for (int i = 0; i < 4; i++) {
            if (IR[1] == valid_second[i]) found_second = true;
        }

        if (!found_first || !found_second) {
            printf("Invalid Instruction: %c%c%c%c\n", IR[0], IR[1], IR[2], IR[3]);
            PI = 1;
            terminate(4);
            return;
        }

        if (IR[0] == 'L' && IR[1] == 'R') {
            int addr = (IR[2]-'0')*10 + (IR[3]-'0');
            if (!(IR[2] >= '0' && IR[2] <= '9' && IR[3] >= '0' && IR[3] <= '9')) {
                printf("Invalid operand\n");
                terminate(3);
                break;
            }

            TTC++;
            if (TTC > TL) {
                TI = 2;
                MOS();
                break;
            }

            int ra = getRA(addr);
            if (PI == 3) {
                handlePageFault();
                PI = 0;
                ra = getRA(addr);
            }
            for (int i = 0; i < 4; i++) R[i] = M[ra][i];
        }

        else if (IR[0] == 'S' && IR[1] == 'R') {
            int addr = (IR[2]-'0')*10 + (IR[3]-'0');
            if (!(IR[2] >= '0' && IR[2] <= '9' && IR[3] >= '0' && IR[3] <= '9')) {
                printf("Invalid operand\n");
                terminate(3);
                break;
            }

            TTC++;
            if (TTC > TL) {
                TI = 2;
                MOS();
                break;
            }

            int ra = getRA(addr);
            if (PI == 3) {
                handlePageFault();
                PI = 0;
                ra = getRA(addr);
            }
            for (int i = 0; i < 4; i++) M[ra][i] = R[i];
        }
        
        else if (IR[0] == 'C' && IR[1] == 'R') {
            int addr = (IR[2]-'0')*10 + (IR[3]-'0');
            if (!(IR[2] >= '0' && IR[2] <= '9' && IR[3] >= '0' && IR[3] <= '9')) {
                printf("Invalid operand\n");
                terminate(3);
                break;
            }
            C = 1;
            int ra = getRA(addr);
            if (PI == 3) {
                handlePageFault();
                PI = 0;
                ra = getRA(addr);
            }
            for (int i = 0; i < 4; i++) {
                if (R[i] != M[ra][i]) {
                    C = 0;
                    break;
                }
            }
            TTC++;
            if (TTC > TL) {
                TI = 2;
                MOS();
                break;
            }
        }

        else if (IR[0] == 'B' && IR[1] == 'T') {
            int addr = (IR[2]-'0')*10 + (IR[3]-'0');
            if (!(IR[2] >= '0' && IR[2] <= '9' && IR[3] >= '0' && IR[3] <= '9')) {
                printf("Invalid operand\n");
                terminate(3);
                break;
            }
            TTC++;
            if (TTC > TL) {
                TI = 2;
                MOS();
                break;
            }

            if (C == 1) IC = addr;
        }
        
        else if (IR[0] == 'G' && IR[1] == 'D') {
            if (!(IR[2] >= '0' && IR[2] <= '9' && IR[3] >= '0' && IR[3] <= '9')) {
                printf("Invalid operand\n");
                terminate(3);
                break;
            }

            TTC++;
            if (TTC > TL) {
                TI = 2;
                MOS();
                break;
            }

            SI = 1;
            MOS();
        }

        else if (IR[0] == 'P' && IR[1] == 'D') {
            if (!(IR[2] >= '0' && IR[2] <= '9' && IR[3] >= '0' && IR[3] <= '9')) {
                printf("Invalid operand\n");
                terminate(3);
                break;
            }
            
            TTC++;
            if (TTC > TL) {
                TI = 2;
                MOS();
                break;
            }

            SI = 2;
            MOS();
        }
        else {
            printf("Invalid instruction: %c%c%c%c\n", IR[0], IR[1], IR[2], IR[3]);
            terminate(4);
            break;
        }        
    }
}

void read() {
    char word[41];
    int addr = (IR[2]-'0')*10 + (IR[3]-'0');

    if (fgets(word, sizeof(word), fin) == NULL) {
        printf("Out of Data: GD tried to read but no data available!\n");
        terminate(0);
        return;
    }

    if (cmpString(word, "$END", 4)) {
        printf("Out of Data: Hit $END while reading data!\n");
        terminate(0);
        return;
    }

    int k = 0;
    for (int i = 0; i < 10; i++) {
        int ra = getRA(addr + i);
        
        if (PI == 3) {
            handlePageFault();
            PI = 0;
            ra = getRA(addr + i);
        }

        for (int j = 0; j < 4; j++) {
            if (word[k] != '\0' && word[k] != '\n' && word[k] != '\r')
                M[ra][j] = word[k++];
            else
                M[ra][j] = ' ';
        }
    }
}

void write() {
    int addr = (IR[2]-'0')*10 + (IR[3]-'0');

    LLC++;
    if (LLC > LL) {
        printf("Line Limit Exceeded !!\n");
        terminate(2);
        return;
    }

    for (int i = 0; i < 10; i++) {
        int ra = getRA(addr + i);
        if (PI == 3) {
            handlePageFault();
            PI = 0;
            ra = getRA(addr + i);
        }
        for (int j = 0; j < 4; j++)
            fputc(M[ra][j], fout);
    }

    fputc('\n', fout);
}

void terminate(int errorCode) {
    fprintf(fout, "\n");
    fprintf(fout, "JOB ID : %s\n", jobID);
    
    switch(errorCode) {
        case 0:
            fprintf(fout, "NO ERROR\n");
            break;
        case 1:
            fprintf(fout, "TIME LIMIT EXCEEDED\n");
            break;
        case 2:
            fprintf(fout, "LINE LIMIT EXCEEDED\n");
            break;
        case 3:
            fprintf(fout, "OPERAND ERROR\n");
            break;
        case 4:
            fprintf(fout, "OPCODE ERROR\n");
            break;
        default:
            fprintf(fout, "NO ERROR\n");
            break;
    }
    
    fprintf(fout, "IC : %d\n", IC);
    fprintf(fout, "IR : %c%c%c%c\n", IR[0], IR[1], IR[2], IR[3]);
    fprintf(fout, "TTC : %d\n", TTC);
    fprintf(fout, "LLC : %d\n", LLC);
    fprintf(fout, "\n");
}

void handlePageFault() {
    printf("%s\n", "page fault occured !!");
    int page = faultVA / 10;
 
    int frame = randomNumberGenerator();
    pageTable[page] = frame;
 
    PI = 0;
}


void MOS() {
    if (TI == 2) {
        printf("Time Limit Exceeded\n");
        terminate(1);
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
        terminate(0);
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