#include<stdio.h>
#include<stdbool.h>
#include "utils.h"
#include "globals.h"
#include<stdlib.h>

// Function to Compare String
bool cmpString(char s1[], char s2[], int size) {
    for (int i = 0; i < size; i++) {
        if (s1[i] != s2[i]) {
            return false;
        }
    }
    return true;
}

// Function to Compare String
void countLimit(char s1[], int* var, int start, int end) {
    for(int i = start; i < end; i++) {
        (*var) = (*var) * 10 + (s1[i] - '0');
    }
}


int lenString(char s[]) {
    int ans = 0;
    while (s[ans] != '\0') {
        ans++;
    }

    return ans;
}

int randomNumberGenerator() {
    for (int i = 0; i < 30; i++) {
        int n = rand() % 30;

        if (!frameUsed[n]) {
            frameUsed[n] = true;
            return n;
        }
    }

    printf("No free frames available!\n");
    exit(0);
}