#include "executor.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definição da memória
uint16_t MEMORIA[258];

// Criação de variáveis globais utilizadas na execução
char AC = 0;
char PC = 0;

int ExecutarNeander() {
  FILE *file = fopen("arquivos/output.mem", "rb");

  if (file == NULL) {
    printf("Erro ao abrir o arquivo");
    return 1;
  }

  size_t items = fread(MEMORIA, 2, 258, file);
  if (items != 258) {
    printf("Erro ao ler o arquivo!\n");
    fclose(file);
    return 1;
  }
  fclose(file);

  int start = 1;

  while (start == 1 && AC < 40) {
    start = read(MEMORIA[PC + OFFSET]);
  }

  FILE *arquivo = fopen("FINAL_COMPILADO.mem", "w");

  if (arquivo == NULL) {
    perror("Erro ao criar o arquivo");
    return 1;
  }

  fwrite(MEMORIA, 2, 258, arquivo);
  fclose(arquivo);

  return 0;
}

// Função responsável pela leitura
int read(uint16_t line) {
  switch (line) {
  case NOP_N:
    printf("NOP\n");
    PC++;
    break;
  case STA_N:
    MEMORIA[MEMORIA[PC + 3] + OFFSET] = AC;
    PC += 2;
    break;
  case LDA_N:
    AC = MEMORIA[MEMORIA[PC + 3] + OFFSET];
    PC += 2;
    break;
  case ADD_N:
    AC += MEMORIA[MEMORIA[PC + 3] + OFFSET];
    PC += 2;
    break;
  case OR_N:
    AC = AC | MEMORIA[MEMORIA[PC + 3] + OFFSET];
    PC += 2;
    break;
  case AND_N:
    AC = AC & MEMORIA[MEMORIA[PC + 3] + OFFSET];
    PC += 2;
    break;
  case NOT_N:
    AC = ~AC;
    PC++;
    break;
  case JMP_N:
    PC = MEMORIA[PC + 3];
    break;
  case JN_N:
    if (AC < 0) {
      PC = MEMORIA[PC + 3];
    } else {
      PC += 2;
    }
    break;
  case JZ_N:
    if (AC == 0) {
      PC = MEMORIA[PC + 3];
    } else {
      PC += 2;
    }
    break;
  case HLT_N:
    printf("AC: %d\n", AC);
    printf("PC: %d\n", PC);
    return 0;
    break;
  default:
    printf("Instrucao nao reconhecida");
    return 0;
    break;
  }

  return 1;
}
