// bfe.c - Interpretador BrainFuck
#include <stdio.h>
#include <stdlib.h>

#define TAPE_SIZE 30000

int main() {
    char tape[TAPE_SIZE] = {0};
    char *ptr = tape;

    char code[100000];
    int code_len = fread(code, 1, sizeof(code) - 1, stdin);
    code[code_len] = '\0';

    for (int pc = 0; pc < code_len; pc++) {
        switch (code[pc]) {
            case '>': ptr++; break;
            case '<': ptr--; break;
            case '+': (*ptr)++; break;
            case '-': (*ptr)--; break;
            case '.': putchar(*ptr); fflush(stdout); break;
            case ',': *ptr = getchar(); break;
            case '[':
                if (*ptr == 0) {
                    int loop = 1;
                    while (loop) {
                        pc++;
                        if (code[pc] == '[') loop++;
                        else if (code[pc] == ']') loop--;
                    }
                }
                break;
            case ']':
                if (*ptr != 0) {
                    int loop = 1;
                    while (loop) {
                        pc--;
                        if (code[pc] == '[') loop--;
                        else if (code[pc] == ']') loop++;
                    }
                }
                break;
        }
    }
    return 0;
}
