#include <stdint.h>

// Código dos mnemônicos
#define NOP_N 0
#define STA_N 16
#define LDA_N 32
#define ADD_N 48
#define OR_N 64
#define AND_N 80
#define NOT_N 96
#define JMP_N 128
#define JN_N 144
#define JZ_N 160
#define HLT_N 240

// Definição do offset pelo header do arquivo
#define OFFSET 2

int read(uint16_t line);
int ExecutarNeander();
