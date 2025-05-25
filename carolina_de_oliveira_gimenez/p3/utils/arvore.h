#ifndef ARVORE__H
#define ARVORE__H

#include <wchar.h>

typedef enum { SOMA, SUB, MUL, DIV, NUMERO, VARIAVEL } TipoNo;

typedef struct no {
    TipoNo tipo;
    union {
        int valor;
        wchar_t operador;
    } dado;
    struct no* esquerda;
    struct no* direita;
} No;

No* criarNoNumero(int valor);
No* criarNoVariavel(wchar_t* variavel);
No* criarNoOperador(TipoNo tipo, wchar_t operador, No* esquerda, No* direita);
void printarArvore(No* raiz);
void freeArvore(No* raiz);

int ehOperacao(No* no);

#endif