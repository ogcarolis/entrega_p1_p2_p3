#include "bfc.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Função principal
int main() {
  setlocale(LC_ALL, "");

  // Alocação de memória para o compilador
  Compilador *comp = (Compilador *)malloc(sizeof(Compilador));

  if (comp == NULL) {
    printf("Erro ao alocar memória para o compilador\n");
    return 1;
  }

  // Alocação de memória para entrada (em wchar_t)
  comp->entrada = (wchar_t *)malloc(sizeof(wchar_t) * BUFFER);

  if (comp->entrada == NULL) {
    printf("Erro ao alocar memória para a entrada\n");
    free(comp);
    return 1;
  }

  // Leitura da entrada padrão (stdin)
  char entrada_str[BUFFER];

  if (fgets(entrada_str, BUFFER, stdin) == NULL) {
    printf("Erro ao ler a entrada\n");
    freeCompilador(comp);
    return 1;
  }

  // Remove o caractere de nova linha, se existir
  entrada_str[strcspn(entrada_str, "\n")] = '\0';

  // Converte string (char*) para wchar_t
  mbstowcs(comp->entrada, entrada_str, BUFFER);

  // Converte string (char*) para wchar_t
  comp->atribuicao = NULL;

  // Processa a gramática da linguagem
  programa(comp);

  // Processa a gramática da linguagem
  gerarBrainFuck(comp);

  // Libera recursos
  freeCompilador(comp);
  return 0;
}

// Avança para o próximo token da entrada
void proximoToken(Compilador *comp) {
  wchar_t c;

  // Ignora espaços em branco (exceto nova linha)
  do {
    c = *(comp->entrada++);
  } while (iswspace(c) && c != L'\n');

  // Verifica fim da entrada
  if (c == L'\0') {
    comp->token.tipo = TOKEN_EOF;
    return;
  }

  // Ignora quebra de linha
  if (c == L'\n') {
    proximoToken(comp);
    return;
  }

  // Reconhece token '='
  if (c == L'=') {
    comp->token.tipo = TOKEN_ASSIGN;
    wcscpy(comp->token.texto, L"=");
    return;
  }

  // Reconhece operadores '+', '-', '*', '/'
  if (wcschr(L"+-*/", c)) {
    comp->token.tipo = TOKEN_OP;
    comp->token.texto[0] = c;
    comp->token.texto[1] = '\0';
    return;
  }

  // Reconhece parênteses '(' e ')'
  if (c == L'(') {
    comp->token.tipo = TOKEN_LPAREN;
    wcscpy(comp->token.texto, L"(");
    return;
  }

  if (c == ')') {
    comp->token.tipo = TOKEN_RPAREN;
    wcscpy(comp->token.texto, L")");
    return;
  }

  // Reconhece identificadores (variáveis)
  if (iswalpha(c)) {
    comp->token.tipo = TOKEN_STRING;
    int i = 0;
    comp->token.texto[i++] = c;
    while (iswalpha(c = *(comp->entrada++)) || iswdigit(c)) {
      comp->token.texto[i++] = c;
    }
    comp->entrada--;
    comp->token.texto[i] = '\0';
    return;
  }

  // Reconhece números (com possível sinal '+' ou '-')
  if (iswdigit(c) || c == L'+' || c == L'-') {
    int i = 0;
    comp->token.texto[i++] = c;
    while (iswdigit(c = *(comp->entrada++))) {
      comp->token.texto[i++] = c;
    }
    comp->entrada--;
    comp->token.texto[i] = L'\0';
    comp->token.tipo = TOKEN_NUM;
    return;
  }

  printf("Token inválido\n");
}

// Verifica se o token atual é o esperado, caso contrário gera erro
void espera(Compilador *comp, TokenTipo tipo, const char *esperado) {
  if (comp->token.tipo != tipo) {
    wprintf(L"Esperado: %s, mas encontrado: %ls\n", esperado,
            comp->token.texto);
    freeCompilador(comp);
    exit(1);
  }
  proximoToken(comp);
}

// <programa> ::= <string>=<expressao>
void programa(Compilador *comp) {

  comp->atribuicao = (Atribuicao *)malloc(sizeof(Atribuicao));

  if (comp->atribuicao == NULL) {
    printf("Erro no malloc de atribuição\n");
    freeCompilador(comp);
    exit(1);
  }
  proximoToken(comp);

  // Captura o nome da variável
  wcscpy(comp->atribuicao->variavel, comp->token.texto);
  espera(comp, TOKEN_STRING, "variável");

  // Captura o nome da variável
  espera(comp, TOKEN_ASSIGN, "=");

  // Processa a expressão
  comp->atribuicao->expressao = expressao(comp);

  // Espera fim da entrada
  espera(comp, TOKEN_EOF, "FIM DO TEXTO");
}

// <expressao> ::= <adicao>
No *expressao(Compilador *comp) { return adicao(comp); }

// <adicao> ::= <multiplicacao> {(+|-) <multiplicacao>}
No *adicao(Compilador *comp) {
  No *retorno = multiplicacao(comp);

  while (comp->token.tipo == TOKEN_OP &&
         (wcscmp(comp->token.texto, L"+") == 0 ||
          wcscmp(comp->token.texto, L"-") == 0)) {
    TipoNo tipo;
    wchar_t simbolo = comp->token.texto[0];

    if (simbolo == L'+')
      tipo = SOMA;
    else
      tipo = SUB;

    proximoToken(comp);
    No *dir = multiplicacao(comp);

    retorno = criarNoOperador(tipo, simbolo, retorno, dir);
  }

  return retorno;
}

// <multiplicacao> ::= <primaria> {(*|/) <primaria>}
No *multiplicacao(Compilador *comp) {
  No *retorno = primaria(comp);

  while (comp->token.tipo == TOKEN_OP &&
         (wcscmp(comp->token.texto, L"*") == 0 ||
          wcscmp(comp->token.texto, L"/") == 0)) {
    TipoNo tipo;
    wchar_t simbolo = comp->token.texto[0];

    if (simbolo == L'*')
      tipo = MUL;
    else
      tipo = DIV;

    proximoToken(comp);
    No *dir = primaria(comp);

    retorno = criarNoOperador(tipo, simbolo, retorno, dir);
  }

  return retorno;
}

// <primaria> ::= NUM | '(' <expressao> ')'
No *primaria(Compilador *comp) {
  if (comp->token.tipo == TOKEN_NUM) {
    No *valor = criarNoNumero((int)wcstol(comp->token.texto, NULL, 0));
    proximoToken(comp);
    return valor;
  } else if (comp->token.tipo == TOKEN_LPAREN) {
    proximoToken(comp);

    No *no = expressao(comp);

    espera(comp, TOKEN_RPAREN, ")");

    return no;
  }

  return NULL;
}

// Gera código Brainfuck para imprimir um caractere Unicode
void gerarChar(wchar_t c) {
  char utf8[MB_CUR_MAX + 1];
  int len = wctomb(utf8, c);

  if (len <= 0)
    return;

  for (int i = 0; i < len; i++) {
    printf("[-]");
    unsigned char byte = (unsigned char)utf8[i];
    for (int j = 0; j < byte; j++) {
      printf("+");
    }
    printf(".");
  }
}

// Gera código Brainfuck para imprimir uma string
void gerarString(const wchar_t *str) {
  for (int i = 0; str[i] != L'\0'; i++) {
    gerarChar(str[i]);
  }
}

// Gera código Brainfuck para uma operação aritmética
void gerarOperacao(No *no) {
  if (no == NULL)
    return;

  if (no->tipo == NUMERO) {
    printf("[-]");
    int valor = no->dado.valor;

    if (valor < 0) {
      valor = -valor;
      for (int i = 0; i < valor; i++) {
        printf("-");
      }
    } else {
      for (int i = 0; i < valor; i++) {
        printf("+");
      }
    }
    return;
  }

  gerarOperacao(no->esquerda);
  printf(">");
  gerarOperacao(no->direita);

  switch (no->tipo) {
  case SOMA:
    // Layout: [a][b][0][result]

    // Soma: result = a + b
    printf("[<+>-]"); // move b para a (posição 0)
    printf("<");      // volta para posição 0 (resultado)
    break;

  case SUB:
    // Layout: [a][b][0][result]

    // Subtração: result = a - b
    printf("[<->-]"); // subtrai b de a
    printf("<");      // volta para posição 0 (resultado)
    break;

  case MUL:
    // Layout: [a][b][resultado][tmp]
    // Algoritmo: Para cada unidade de 'a', adiciona 'b' ao resultado

    printf(">[-]>[-]<<"); // limpa resultado e tmp
    printf("<");          // volta para 'a'
    printf("[>");         // enquanto a > 0, move para 'b'

    printf("[->+>+<<]"); // copia 'b' para tmp1 e tmp2

    printf(">>");       // move para tmp2
    printf("[-<<+>>]"); // restaura 'b' usando tmp2

    printf("<<<-]"); // decrementa 'a' e fecha loop
    printf(">>");    // move para posição do resultado
    printf("[<<+>>-]");
    printf("<[-]<");
    break;

  case DIV:
    // Layout: [a][b][0][1][0][tmp][resultado]
    // Algoritmo: Subtrai b de a repetidamente, incrementando o quociente

    printf(">[-]>[-]>[-]>[-]>[-]<<<<<<"); // limpa todas as células e volta para
                                          // 'a'
    printf(">>>+<<<");              // inicializa célula do 1 e volta para 'a'
    printf("[>->>>>+<<<<");         // subtrai b de a e copia a
    printf("[>]>>");                // move se a >= 0
    printf("[>>>+<[<<<<+>>>>-]<]"); // incrementa quociente
    printf("<<<<-]");               // fecha loop principal
    printf(">>>>>>");               // move para quociente
    printf("[<<<<<<+>>>>>>-]");     // move resultado para célula inicial
    printf("<[-]<<-<<[-]<");        // limpa células temporárias
    break;
  }
}

// Gera todo o código Brainfuck a partir da árvore
void gerarBrainFuck(Compilador *comp) {
  gerarString(comp->atribuicao->variavel);


  gerarChar(L'=');
  gerarOperacao(comp->atribuicao->expressao);

  printf(">[-]>[-]+>[-]+<[>[-<-<<[->+>+<<]>[-<+>]>>]");
  printf("++++++++++>[-]+>[-]>[-]>[-]<<<<<");
  printf("[->-[>+>>]>[[-<+>]+>+>>]<<<<<]");
  printf(">>-[-<<+>>]");
  printf("[->[-]>>+>+<<<]");
  printf("<[-]++++++++[-<++++++>]");
  printf(">>[-<<+>>]<<]");
  printf("<[.[-]<]<");
  printf(">++++++++++.");
}

void freeCompilador(Compilador *comp) {
  if (comp == NULL)
    return;

  if (comp->atribuicao) {
    freeArvore(comp->atribuicao->expressao);
    free(comp->atribuicao);
  }

  free(comp);
  comp = NULL;
}