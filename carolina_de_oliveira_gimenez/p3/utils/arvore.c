#include "arvore.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

No* criarNoNumero(int valor) {
    No* novo_no = (No*)malloc(sizeof(No));
    if (!novo_no) {
        printf("Erro ao alocar memória para o nó da árvore.\n");
        return NULL;
    }
    novo_no->tipo = NUMERO;
    novo_no->dado.valor = valor;
    novo_no->esquerda = NULL;
    novo_no->direita = NULL;
    return novo_no;
}

No* criarNoOperador(TipoNo tipo, wchar_t operador, No* esquerda, No* direita) {
    No* novo_no = (No*)malloc(sizeof(No));
    if (!novo_no) {
        printf("Erro ao alocar memória para o nó da árvore.\n");
        return NULL;
    }
    novo_no->tipo = tipo;
    novo_no->dado.operador = operador;
    novo_no->esquerda = esquerda;
    novo_no->direita = direita;
    return novo_no;
}

void freeArvore(No* raiz) {
    if (raiz == NULL) return;

    freeArvore(raiz->esquerda);
    freeArvore(raiz->direita);

    free(raiz);
}

void _printarArvoreRec(No* raiz, int nivel) {
    if (raiz == NULL) return;

    // Imprime os tabs para o nível atual
    for (int i = 0; i < nivel; i++) {
        printf("\t");
    }

    if (raiz->tipo == NUMERO) {
        printf("Número: %d\n", raiz->dado.valor);
    } else {
        wprintf(L"Operador: %lc\n", raiz->dado.operador);
        _printarArvoreRec(raiz->esquerda, nivel + 1);
        _printarArvoreRec(raiz->direita, nivel + 1);
    }
}

void printarArvore(No* raiz) {
    _printarArvoreRec(raiz, 0);
}

int ehOperacao(No* no) {
    if (no == NULL) return 0;
    return no->tipo == SOMA || no->tipo == SUB || no->tipo == MUL || no->tipo == DIV;
}