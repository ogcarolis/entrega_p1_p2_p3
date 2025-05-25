#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "executor.h"

// Definição de memória e arquivo
uint16_t MEMORIA_ASM[258];

int ExecutarAssembler() {
    FILE *file = fopen("arquivos/output.asm", "r+");

    if (file == NULL) {
        printf("Não foi possível abrir o arquivo.\n");
        return 1;
    }

    Lexer lexer = {file, 1, NULL, NULL, 0};

    // Adição dos headers do arquivo
    MEMORIA_ASM[0] = 0x4e03;
    MEMORIA_ASM[1] = 0x5244;

    // Execução do Lexer
    if (!executaLexer(&lexer)) {
        return 1;
    }

    // Print do lexer
    imprimeLexer(&lexer);

    // Limpeza do lexer
    freeLexer(&lexer);

    // Arquivo de salvamento do bytecode
    FILE *arquivo = fopen("arquivos/output.mem", "wb");

    if (arquivo == NULL) {
        perror("Erro ao criar o arquivo");
        return 1;
    }

    // Salvando o conteúdo do array no arquivo
    fwrite(MEMORIA_ASM, 2, 258, arquivo);
    fclose(arquivo);

    ExecutarNeander();

  return 0;
}

// Função responsável por orquestrar o lexer
int executaLexer(Lexer *lexer) {
    // Primeiro busca o primeiro "."
    if (!encontrarPonto(lexer)) {
        freeLexer(lexer);
        return 0;
    }

    // Encontrando-o verifica o bloco de DATA (que deve vir antes do CODE)
    if (!verificarBloco(lexer, "DATA")) {
        freeLexer(lexer);
        return 0;
    }

    // Le e guarda no lexer as variáveis encontradas em .DATA
    if (!lerData(lexer)) {
        printf("\033[1;41;30mERRO:\033[0m ao ler o bloco data na linha %d\n", lexer->linha);
        freeLexer(lexer);
        return 0;
    }

    // Verifica o bloco de .CODE
    if(!verificarBloco(lexer, "CODE")) {
        freeLexer(lexer);
        return 0;
    }

    // Busca o próximo "."
    if (!encontrarPonto(lexer)) {
        freeLexer(lexer);
        return 0;
    }

    // Verifica a origem após o próximo "." encontrado e guarda a posição no lexer (ex: .ORG 0 -> lexer->origem = 0)
    if (!lerOrigem(lexer)) {
        freeLexer(lexer);
        return 0;
    }

    // Le as instruções do bloco de código, guardando-as no Lexer
    if (!lerInstrucoes(lexer)) {
        freeLexer(lexer);
        return 0;
    }

    // Salva na memória as variáveis no bloco de .DATA
    if (!salvarData(lexer)) {
        printf("\033[1;41;30mERRO:\033[0m ao salvar o bloco DATA na memória\n");
        freeLexer(lexer);
        return 0;
    }
    
    // Salva na memória as instruções no bloco de .CODE
    if (!salvarCode(lexer)) {
        printf("\033[1;41;30mERRO:\033[0m ao salvar o bloco CODE na memória\n");
        freeLexer(lexer);
        return 0;
    }

    return 1;
}

// Le o comentário e pula a linha
int proxLinha(Lexer* lexer) {
  char c;
  while((c = fgetc(lexer->file)) != -1) {
      if (c == '\n') {
          lexer->linha++;
          return 1;
      }
  }

  return 0;
}

// Busca o primeiro ponto em seguida da posição atual no arquivo
int encontrarPonto(Lexer* lexer) {
  char c;
  while((c = fgetc(lexer->file)) != -1) {
      if (c == '.') return 1;
      else if (c == '\n') {
          lexer->linha++;
      }
      else if (c == COMENTARIO && !proxLinha(lexer)) {
          c = -1;
          break;
      }
      else if (c != COMENTARIO && c != SPACE && c != TAB) {
          printf("\033[1;41;30mERRO:\033[0m Caractere invalido \'%c\' na linha %d.\n", c, lexer->linha);
          return 0;
      }
  }

  return 0;
}

// Verifica se o bloco é o buscado (ex: CODE/DATA)
int verificarBloco(Lexer* lexer, char* bloco) {
  Palavra palavra = lerPalavra(lexer);
  
  if (strcmp(palavra.texto, bloco) != 0) {
      printf("\033[1;41;30mERRO:\033[0m Estrutura desconhecida %s (não achado o .%s) na linha %d\n", palavra.texto, bloco, lexer->linha);
      return 0;
  }
  
  if (palavra.ultimo == COMENTARIO && proxLinha(lexer)) {
      printf("\033[1;41;30mERRO:\033[0m Fim do arquivo\n");
      return 0;
  }
  else if (palavra.ultimo == '\n') {
      lexer->linha++;
  }
  else if (palavra.ultimo == SPACE || palavra.ultimo == TAB) {
      char c;
      while((c = fgetc(lexer->file)) != -1) {
          if (c == '\n') {
              lexer->linha++;
              break;
          }
          else if (c == COMENTARIO && !proxLinha(lexer)) {
              c = -1;
              break;
          }
          else if (c != COMENTARIO && c != SPACE && c != TAB) {
              printf("\033[1;41;30mERRO:\033[0m Caractere inválido \'%c\' na linha %d.\n", c, lexer->linha);
              return 0;
          }
      }

      if (c == -1) {
          printf("\033[1;41;30mERRO:\033[0m Arquivo incompleto.\n");
          return 0;
      }
  }
  else if (palavra.ultimo != COMENTARIO) {
      printf("\033[1;41;30mERRO:\033[0m caractere inválido (\'%c\') depois da estrutura %s %d\n", palavra.ultimo, bloco, lexer->linha);
      return 0;
  }

  return 1;
}

// Le o conteúdo do bloco de .DATA
int lerData(Lexer* lexer) {
  char c;
  int index;
  Data* nova = NULL;

  while((c = fgetc(lexer->file)) != -1) {

      if (c == COMENTARIO && !proxLinha(lexer)) {
        printf("\033[1;41;30mERRO:\033[0m Falha ao processar comentário na linha %d.\n", lexer->linha);
        return 0;
      }
      else if (ehLetra(c)) {
          index = 1;
          nova = (Data*) malloc(sizeof(Data));

          if (nova == NULL) {
              printf("\033[1;41;30mERRO:\033[0m Não foi possível alocar na memória.\n");
              return 0;
          }

          nova->prox = lexer->data;
          nova->token[0] = c;

          // Pegar nome de variável
          do {
              c = fgetc(lexer->file);
              if (c == COMENTARIO || c == '\n') {
                  free(nova);
                  return 1;
              }
              else if (c != SPACE && c != TAB) {
                  nova->token[index] = c;
                  index++;
              }
          } while(c != -1 && c != TAB && c != SPACE && index < TOKEN_SIZE);

          if (c == -1) {
            printf("\033[1;41;30mERRO:\033[0m Fim inesperado do arquivo na linha %d.\n", lexer->linha);
            free(nova);
            return 0;
          }

          nova->token[index] = '\0';

          // Pegar o tipo
          Palavra palavra = lerPalavra(lexer);

          if (palavra.ultimo != SPACE && palavra.ultimo != TAB) {
            printf("\033[1;41;30mERRO:\033[0m Tipo de variável inválido na linha %d.\n", lexer->linha);
            free(nova);
            return 0;
          }

          strncpy(nova->tipo, palavra.texto, palavra.tamanho);

          // Pegar o valor
          palavra = lerPalavra(lexer);

          if (palavra.ultimo != SPACE && palavra.ultimo != TAB && palavra.ultimo != '\n' && palavra.ultimo != COMENTARIO) {
            printf("\033[1;41;30mERRO:\033[0m Formato de valor inválido na linha %d.\n", lexer->linha);
            free(nova);
            return 0;
          }

          if (palavra.tamanho > 2) {
            printf("\033[1;41;30mERRO:\033[0m Valor hexadecimal muito longo na linha %d.\n", lexer->linha);
            free(nova);
            return 0;
          }

          // Valor pode ser um Hexadecimal ou Indefinido (?)  
          if (
              (!ehHexadecimal(palavra.texto[0]) && palavra.texto[0] != '?') || 
              (palavra.tamanho == 2 && !ehHexadecimal(palavra.texto[1]))
          ) {
            printf("\033[1;41;30mERRO:\033[0m Valor inválido na linha %d.\n", lexer->linha);
            free(nova);
            return 0;
          }
          
          // Caso o valor seja indefinido, define a variável como vazia, se não, guarda o valor
          if (palavra.texto[0] == '?') {
              nova->vazio = 1;
          }
          else {
              nova->valor = (uint8_t)strtol(palavra.texto, NULL, 16);
              nova->vazio = 0;
          }
          

          if (palavra.ultimo == COMENTARIO && !proxLinha(lexer)) {
              return 1;
          }
          else if (palavra.ultimo == '\n') {
              lexer->linha++;
          }
          else if (c != COMENTARIO) {
              do {
                  c = fgetc(lexer->file);
                  if (c != SPACE && c != TAB && c != COMENTARIO && c != '\n') {
                    printf("\033[1;41;30mERRO:\033[0m Caractere inesperado (\'%c\') na linha %d.\n", c, lexer->linha);
                    free(nova);
                    return 0;
                  }
              } while(c != -1 && c != COMENTARIO && c != '\n');
  
              if (c == -1) {
                printf("\033[1;41;30mERRO:\033[0m Fim inesperado do arquivo na linha %d.\n", lexer->linha);
                free(nova);
                return 0;
              }

              if (c == COMENTARIO && !proxLinha(lexer)) {
                printf("\033[1;41;30mERRO:\033[0m Falha ao processar comentário na linha %d.\n", lexer->linha);
                free(nova);
                return 0;
              }
              else if (c == '\n') {
                lexer->linha++;
              }
          }
          
          lexer->data = nova;
      }
      else if (c == '\n') lexer->linha++;
      else if (c == '.') return 1; // Retorna 1 caso encontrado o próximo . após todas as definições
      else if (c != TAB && c != SPACE) {
        printf("\033[1;41;30mERRO:\033[0m Caractere inesperado \'%c\' na linha %d.\n", c, lexer->linha);
        return 0; 
      }
  }

  printf("\033[1;41;30mERRO:\033[0m Fim inesperado do arquivo antes do próximo ponto na linha %d.\n", lexer->linha);
  return 0;
}

// Le e salva cada palavra antes do ESPAÇO ou TAB
Palavra lerPalavra(Lexer* lexer) {
  Palavra palavra;
  palavra.tamanho = 0;
  palavra.ultimo = '\0';

  char c;

  while((c = fgetc(lexer->file)) != -1) {
      if (c == '\n' || c == TAB || c == SPACE || c == COMENTARIO || palavra.tamanho == TOKEN_SIZE) {
          palavra.ultimo = c;
          palavra.texto[palavra.tamanho] = '\0';
          return palavra;
      }
      
      palavra.texto[palavra.tamanho] = c;
      palavra.tamanho++;
  }

  palavra.texto[0] = '\0';

  return palavra;
}

// Salva o token de cada Mnemonico
Mnemonico salvarMnemonico(char* token) {
  if (strcmp(token, "NOP") == 0) return NOP;
  else if (strcmp(token, "STA") == 0) return STA;
  else if (strcmp(token, "LDA") == 0) return LDA;
  else if (strcmp(token, "ADD") == 0) return ADD;
  else if (strcmp(token, "AND") == 0) return AND;
  else if (strcmp(token, "OR") == 0) return OR;
  else if (strcmp(token, "NOT") == 0) return NOT;
  else if (strcmp(token, "JMP") == 0) return JMP;
  else if (strcmp(token, "JN") == 0) return JN;
  else if (strcmp(token, "JZ") == 0) return JZ;
  else if (strcmp(token, "HLT") == 0) return HLT;
  else return DESCONHECIDO;
}

// Tokens que não precisam de atribuição de valor
int NaoPossuiValor(Mnemonico m) {
  return m == NOP || m == NOT || m == HLT || m == DESCONHECIDO;
}

// Guarda no lexer o valor da origem do arquivo, verificando se há esta definição
int lerOrigem(Lexer* lexer) {
  Palavra palavra = lerPalavra(lexer);

  if (strcmp(palavra.texto, "ORG") != 0) {
      printf("\033[1;41;30mERRO:\033[0m estrutura errada depois de .CODE %s (não encontrado o .ORG) na linha %d\n", palavra.texto, lexer->linha);
      return 0;
  }
  
  if (palavra.ultimo != SPACE && palavra.ultimo != TAB) {
      printf("\033[1;41;30mERRO:\033[0m valor de origem nao achado na linha %d\n", lexer->linha);
      return 0;
  }

  palavra = lerPalavra(lexer);

  if (palavra.tamanho > 2 || !ehHexadecimal(palavra.texto[0]) || (palavra.tamanho == 2 && !ehHexadecimal(palavra.texto[1]))) {
      printf("\033[1;41;30mERRO:\033[0m Posição invalida na linha %d\n", lexer->linha);
      return 0;
  }

  lexer->origem = (uint8_t)strtol(palavra.texto, NULL, 16);

  if (palavra.ultimo == '\n') lexer->linha++;
  else if (palavra.ultimo == COMENTARIO && !proxLinha(lexer)) {
      printf("\033[1;41;30mERRO:\033[0m Fim do arquivo\n");
      return 0;
  }
  else if (palavra.ultimo != COMENTARIO && palavra.ultimo != SPACE && palavra.ultimo != TAB) {
      printf("\033[1;41;30mERRO:\033[0m Caractere invalido \'%c\' na linha %d\n", palavra.ultimo, lexer->linha);
      return 0;
  }

  return 1;
}

// Le as instruções e guarda no lexer, verificando se estão conforme requisitos necessários
int lerInstrucoes(Lexer* lexer) {
  char c;
  int index;
  Instrucao* nova = NULL;

  while((c = fgetc(lexer->file)) != -1) {
      if (c == COMENTARIO && !proxLinha(lexer)) {
          printf("\033[1;41;30mERRO:\033[0m Fim inesperadodo arquivo na linha %d\n", lexer->linha);
          return 0;
      }
      else if (ehLetra(c)) {
          nova = (Instrucao*) malloc(sizeof(Instrucao));

          if (nova == NULL) {
              printf("\033[1;41;30mERRO:\033[0m Falha ao alocar memória na linha %d\n", lexer->linha);
              return 0;
          }

          nova->prox = NULL;
          Instrucao* aux = lexer->code;

          while(aux != NULL && aux->prox != NULL) {
              aux = aux->prox;
          }

          char mnemonico[MAX_MNEMONICO_SIZE + 1];
          mnemonico[0] = c;

          index = 1;
          // pegar mnemonico
          do {
              c = fgetc(lexer->file);
              if (ehLetra(c)) {
                  mnemonico[index] = c;
                  index++;
              }
              else if (c != -1 && c != SPACE && c != TAB && c != '\n') {
                  printf("\033[1;41;30mERRO:\033[0m Mnemonico invalido na linha %d\n", lexer->linha);
                  free(nova);                  
                  return 0;
              }
          } while(c != -1 && c != TAB && c != SPACE && c != '\n' && index < MAX_MNEMONICO_SIZE + 1);

          mnemonico[index] = '\0';

          nova->mnemonico = salvarMnemonico(mnemonico);

          if (nova->mnemonico == DESCONHECIDO) {
              printf("\033[1;41;30mERRO:\033[0m Mnemonico desconhecido \'%s\' na linha %d\n", mnemonico, lexer->linha);
              free(nova);
              return 0;
          }

          if (!NaoPossuiValor(nova->mnemonico)) {
              // pegar valor
              Palavra palavra = lerPalavra(lexer);

              if (palavra.tamanho == 0) {
                  printf("\033[1;41;30mERRO:\033[0m O mnemonico \'%s\' precisa de um complemento na linha %d\n", mnemonico, lexer->linha);
                  free(nova);
                  return 0;
              }

              strncpy(nova->token, palavra.texto, palavra.tamanho);
              nova->token[palavra.tamanho] = '\0';

              if (palavra.ultimo == COMENTARIO && !proxLinha(lexer)) {
                  printf("\033[1;41;30mERRO:\033[0m Fim do arquivo inesperado na linha %d\n", lexer->linha);
                  return 1;
              }
              else if (palavra.ultimo == '\n') {
                  lexer->linha++;
              }
              else if (palavra.ultimo == SPACE || palavra.ultimo == TAB) {
                  do {
                      c = fgetc(lexer->file);
                      if (c != SPACE && c != TAB && c != COMENTARIO && c != '\n') {
                          printf("\033[1;41;30mERRO:\033[0m %c muitos argumentos na linha %d\n", c, lexer->linha);
                          free(nova);
                          return 0;
                      }
                  } while(c != -1 && c != COMENTARIO && c != '\n');
      
                  if (c == -1) {
                      printf("\033[1;41;30mERRO:\033[0m Fim do arquivo inesperado na linha %d\n", lexer->linha);
                      free(nova);
                      return 0;
                  }

                  if (c == COMENTARIO && !proxLinha(lexer)) {
                      printf("\033[1;41;30mERRO:\033[0m Fim do arquivo inesperado na linha %d\n", lexer->linha);
                      free(nova);
                      return 0;
                  }
                  else if (c == '\n') {
                      lexer->linha++;
                  }
              }
              else if (palavra.ultimo != COMENTARIO) {
                  printf("\033[1;41;30mERRO:\033[0m Erro de sintaxe na linha %d\n", lexer->linha);
                  free(nova);
                  return 0;
              }
          }
          else {
              nova->token[0] = '\0';
          }
          
          if (aux == NULL) {
            lexer->code = nova;
          }
          else {
            aux->prox = nova;
          }
      }
      else if (c == '\n') lexer->linha++;
      else if (c != TAB && c != SPACE) {
        printf("\033[1;41;30mERRO:\033[0m Caracter inesperado \'%c\' na linha %d\n", c, lexer->linha);
        return 0;
      }
  }

  return 1;
}

// Imprime o que foi salvado pelo lexer no console
void imprimeLexer(Lexer* lexer) {
  printf("Linha: %d\n", lexer->linha);

  printf("Data:\n");
  Data* auxData = lexer->data;
  while (auxData != NULL) {
      if (auxData->vazio) {
          printf("- %s %s %s\n", auxData->token, auxData->tipo, "Sem valor");
      }
      else {
          printf("- %s %s %2x\n", auxData->token, auxData->tipo, auxData->valor);
      }
      auxData = auxData->prox;
  }

  printf("Code (%x):\n", lexer->origem);
  Instrucao* auxInstrucao = lexer->code;
  while (auxInstrucao != NULL) {
      printf("- %x %s\n", auxInstrucao->mnemonico, auxInstrucao->token);
      auxInstrucao = auxInstrucao->prox;
  }
}

// Realiza a limpeza do lexer
void freeLexer(Lexer* lexer) {
  if (lexer == NULL) return;

  while (lexer->data != NULL) {
      Data* aux = lexer->data->prox;
      free(lexer->data);
      lexer->data = aux;
  }

  while (lexer->code != NULL) {
      Instrucao* aux = lexer->code->prox;
      free(lexer->code);
      lexer->code = aux;
  }

  if (lexer->file)
      fclose(lexer->file);
}

// Conta o tamanho do bloco de instruções para posicionamento de memória
int contarInstrucoes(Lexer* lexer) {
  Instrucao* instrucao = lexer->code;
  int count = 0;

  while (instrucao != NULL) {
      count++;
      if (instrucao->mnemonico == HLT) {
          break;
      }
      else if (!NaoPossuiValor(instrucao->mnemonico)) {
        count++;
      }

      instrucao = instrucao->prox;
  }

  return count;
}

// Conta o bloco de data (quantidade de espaços de memória) 
int contarData(Lexer* lexer) {
    Data* data = lexer->data;
    int count = 0;
  
    while (data != NULL) {
        count++;          
        data = data->prox;
    }
  
    return count;
  }

// Salva na memória as variáveis
int salvarData(Lexer* lexer) {
    Data* auxData = lexer->data;

    int tamanhoCode = contarInstrucoes(lexer);
    int tamanhoData = contarData(lexer);
    uint16_t posicao = (uint16_t)(tamanhoCode + OFFSET + lexer->origem);

    if (tamanhoData + tamanhoCode < 258) {
        while (auxData != NULL) {
            if (!auxData->vazio && tamanhoCode + posicao + 1 < 258) {
                MEMORIA_ASM[posicao] = (uint8_t)auxData->valor;
                auxData->posMemoria = posicao;
                posicao++;
            }
            else if (auxData->vazio && tamanhoCode + posicao + 1 < 258) {
                MEMORIA_ASM[posicao] = 0;
                auxData->posMemoria = posicao;
                posicao++;
            }
            else if (tamanhoCode + posicao + 1 >= 258) {
                printf("\033[1;41;30mERRO:\033[0m Tentativa de escrita fora do limite do arquivo!\n");
                return 0;
            }
            auxData = auxData->prox;
        }
    }
    else {
        printf("\033[1;41;30mERRO:\033[0m Não há espaço para a escrita de variaveis!\n");
        return 0;
    }

    return 1;
}

// Salva na memória as instruções
int salvarCode(Lexer* lexer) {
  Instrucao* auxInstrucao = lexer->code;

  int tamanhoCode = contarInstrucoes(lexer);
  uint16_t dataPos = (uint16_t)(tamanhoCode + OFFSET - 1);
  uint16_t posicao = lexer->origem;

  while (auxInstrucao != NULL) {
      if (posicao + OFFSET < 258) {
          MEMORIA_ASM[posicao + OFFSET] = (uint16_t)auxInstrucao->mnemonico;
          posicao++;

          if(!NaoPossuiValor(auxInstrucao->mnemonico)) {
            size_t tamanho = strlen(auxInstrucao->token);

            Data* auxData = lexer->data;
            uint16_t posicaoVar = dataPos;
            int encontrou = 0;

            while (auxData != NULL) {
              if (strcmp(auxData->token, auxInstrucao->token) == 0) {
                MEMORIA_ASM[posicao + OFFSET] = (uint16_t)auxData->posMemoria - OFFSET;
                posicao++;
                encontrou = 1;
                break;
              }
              posicaoVar++;
              auxData = auxData->prox;
            }

            if (tamanho <= 2 && ehHexadecimal(auxInstrucao->token[0]) && (tamanho != 2 || ehHexadecimal(auxInstrucao->token[1])) && !encontrou) {
              MEMORIA_ASM[posicao + OFFSET] = (uint8_t)strtol(auxInstrucao->token, NULL, 16);
              posicao++;
            }
          }
      }
      else if (posicao + OFFSET >= 258) {
          printf("\033[1;41;30mERRO:\033[0m Tentativa de escrita fora do limite do arquivo!\n");
          return 0;
      }
      auxInstrucao = auxInstrucao->prox;

  }

  return 1;
}

int ehLetra(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
int ehNumero(char c) { return c >= '0' && c <= '9'; }
int ehAlfaNumerico(char c) { return ehLetra(c) || ehNumero(c); }
int ehHexadecimal(char c) { return ehNumero(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }