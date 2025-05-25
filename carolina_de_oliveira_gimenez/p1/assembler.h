#include <stdint.h>
#include <stdio.h>

// Definição do offset por causa do header
#define OFFSET 2

// Definição de comentário, espaço, tab, máximo tamanho do mnemonico e máximo tamanho da variável
#define COMENTARIO ';'
#define SPACE ' '
#define TAB '\t'
#define MAX_MNEMONICO_SIZE 3
#define TOKEN_SIZE 10

// Enum para cada mnemonico (instrução)
typedef enum {
    NOP = 0x0,
    STA = 0x10,
    LDA = 0x20,
    ADD = 0x30,
    OR = 0x40,
    AND = 0x50,
    NOT = 0x60,
    JMP = 0x80,
    JN = 0x90,
    JZ = 0xA0,
    HLT = 0xF0,
    DESCONHECIDO = 0xFF // CASO NÃO EXISTA
} Mnemonico;

// Struct de instruções
typedef struct instrucao {
    Mnemonico mnemonico;
    char token[TOKEN_SIZE + 1];
    struct instrucao* prox;
} Instrucao;

// Struct de Data
typedef struct data {
    char token[TOKEN_SIZE + 1];
    char tipo[TOKEN_SIZE + 1];
    uint8_t valor;
    int vazio;
    struct data* prox;
    uint8_t posMemoria;
} Data;

// Struct do lexer que guarda todo o conteúdo que o lexer converteu
typedef struct lexer {
    FILE* file;
    int linha;
    Data* data;
    Instrucao* code;
    uint8_t origem;
} Lexer;

// Struct de palavras
typedef struct palavra {
    char texto[TOKEN_SIZE + 1];
    char ultimo;
    size_t tamanho;
} Palavra;

// Struct de variáveis
typedef struct variavel {
    char token[TOKEN_SIZE + 1];
    uint16_t pos;
} Variavel;

// Leitura e verificação do arquivo
int proxLinha(Lexer* lexer);
int encontrarPonto(Lexer* lexer);
Palavra lerPalavra(Lexer* lexer);
int verificarBloco(Lexer* lexer, char* bloco);
int lerData(Lexer* lexer);
int lerOrigem(Lexer* lexer);
int lerInstrucoes(Lexer* lexer);
int contarInstrucoes(Lexer* lexer);

// Salvar dados e código no arquivo
int salvarData(Lexer* lexer);
int salvarCode(Lexer* lexer);

// Execução do lexer
void freeLexer(Lexer* lexer);
void imprimeLexer(Lexer* lexer);
int executaLexer(Lexer *lexer);

// Funções de verificação de letra, número, alfa numérico e hexadecimal
int ehLetra(char c);
int ehNumero(char c);
int ehAlfaNumerico(char c);
int ehHexadecimal(char c);

int ExecutarAssembler();