#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Lista de variaveis
typedef struct variavelComp {
    char *nome; 
    char *valor; 
    int tem_valor;
    struct variavelComp *prox;
} VariavelComp;

// Lista auxiliar para armazenar nível das variáveis

/*
Nivel 1 -> variáveis com valor numerico
Nível 2 -> variáveis com expressões com variáveis de nível 1
Nível 3 -> variáveis com expressões com variáveis de nível 2
...
etc.
*/

typedef struct varNivel {
    VariavelComp *var;
    int nivel;
    struct varNivel *prox;
} VarNivel;

int validaLinhaPrograma(char *linha);
int ehNumericoPuro(const char *expr);
int complementoDeDois(int num);

void addVar(const char *nome, const char *valor);
VariavelComp* ordenarVariaveis();

int validaExpressao(char *expr);
char *processaExpressao(char *exprOriginal);
int expressaoSimplesNumero(char *expr);
void gerarInstrucaoExpr(FILE *saida, const char *expr, const char *destino);

int leituraCodigo(FILE *entrada);

void gerarDataCode(FILE *saida);
void gerarCode(FILE *saida);

int execucaoLpnAsm(FILE *entrada);
int ExecutarCompilador();