#include "assembler.h"
#include "compilador.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

VariavelComp *variaveis = NULL;

int ExecutarCompilador() {
    FILE *entrada = fopen("arquivos/programa.lpn", "r");
    if (!entrada) {
        perror("Erro ao abrir arquivo .lpn");
        return 1;
    }

    int executa = execucaoLpnAsm(entrada);
    if (!executa) {
        fclose(entrada);
        return 1;
    }

    fclose(entrada);

    ExecutarAssembler();

    return 0;
}

int ehNumericoPuro(const char *expr) {
    while (*expr) {
        if (!isdigit(*expr) && *expr != '-' && *expr != '+' && *expr != ' ') {
            return 0;
        }
        expr++;
    }
    return 1;
}

int complementoDeDois(int num) {
    return ~abs(num) + 1;
}

int validaExpressao(char *expr) {
    int i = 0;

    // Primeira parte da expressão: deve começar com variável ou número
    if (!isalpha(expr[i]) && !isdigit(expr[i])) {
        return 0;
    }

    // Ler variável ou número
    while (isalnum(expr[i])) {
        i++;
    }

    while (1) {
        // Ignorar os espaços
        while (expr[i] == ' ') i++;

        // Se chegou no fim da string, expressão é válida
        if (expr[i] == '\0') {
            return 1;
        }

        // Agora deve ter + ou -
        if (expr[i] != '+' && expr[i] != '-') {
            return 0;
        }
        i++;

        // Ignorar os espaços após
        while (expr[i] == ' ') i++;

        // Depois de + ou -, deve ter número ou variável
        if (!isalpha(expr[i]) && !isdigit(expr[i])) {
            return 0;
        }

        // Ler variável ou número
        while (isalnum(expr[i])) {
            i++;
        }
    }
}

int ehExpressao(char *valor) {
    if (valor == NULL) return 0;

    int temOperador = 0;
    
    for (int i = 0; valor[i] != '\0'; i++) {
        char c = valor[i];
        
        if (!((c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') ||
              (c == '+') || (c == '-') ||
              (c == ' '))) {
            return 0; // achou caractere inválido
        }

        if (c == '+' || c == '-') {
            temOperador = 1; // encontrou pelo menos um operador
        }
    }

    return temOperador; // só é válido se tiver operador
}

void addVar(const char *nome, const char *valor) {
    // Verifica se já existe
    VariavelComp *atual = variaveis;
    while (atual) {
        if (strcmp(atual->nome, nome) == 0) return;  // Caso a variável já exista ele retorna um erro
        atual = atual->prox;
    }

    printf("%s: %s\n", nome, valor);

    VariavelComp *nova = malloc(sizeof(VariavelComp));
    nova->nome = strdup(nome); 
    nova->tem_valor = (valor != NULL);
    nova->valor = valor ? strdup(valor) : NULL;
    nova->prox = variaveis;
    variaveis = nova;
}

VariavelComp* ordenarVariaveis() {
    VariavelComp *ordenado_inicio = NULL, *ordenado_fim = NULL;
    VariavelComp *v;

    VarNivel *varNiveis = NULL;

    // Inicializa a lista auxiliar
    for (v = variaveis; v; v = v->prox) {
        VarNivel *novo = malloc(sizeof(VarNivel));
        novo->var = v;
        novo->nivel = -1;
        novo->prox = varNiveis;
        varNiveis = novo;
    }

    // Classifica níveis
    int progresso;
    do {
        progresso = 0;
        VarNivel *vn = varNiveis;
        while (vn) {
            if (vn->nivel != -1) {
                vn = vn->prox;
                continue;
            }

            if (!vn->var->tem_valor) {
                vn->nivel = 0;
                progresso = 1;
            }
            else {
                if (ehNumericoPuro(vn->var->valor)) {
                    vn->nivel = 1;
                    progresso = 1;
                } else {
                    // Verifica se depende apenas de nível 1
                    int depende_nivel_maior = 0;
                    char copia[256];
                    strcpy(copia, vn->var->valor);
                    char *token = strtok(copia, "+- ");
                    while (token) {
                        if (isalpha(token[0])) {
                            VarNivel *busca = varNiveis;
                            int achou = 0;
                            while (busca) {
                                if (strcmp(busca->var->nome, token) == 0) {
                                    achou = 1;
                                    if (busca->nivel == -1) {
                                        depende_nivel_maior = -1; // Ainda não classificado
                                    } else if (busca->nivel > 1) {
                                        depende_nivel_maior = 1;
                                    }
                                    break;
                                }
                                busca = busca->prox;
                            }
                            if (!achou) depende_nivel_maior = 1;
                        }
                        token = strtok(NULL, "+- ");
                    }
                    if (depende_nivel_maior == -1) {
                        vn = vn->prox;
                        continue;
                    } else if (depende_nivel_maior == 0) {
                        vn->nivel = 2;
                        progresso = 1;
                    } else {
                        vn->nivel = 3;
                        progresso = 1;
                    }
                }
            }
            vn = vn->prox;
        }
    } while (progresso);

    // Agora cria a lista ordenada
    for (int nivelAtual = 0; nivelAtual <= 3; nivelAtual++) {
        VarNivel *vn = varNiveis;
        while (vn) {
            if (vn->nivel == nivelAtual) {
                VariavelComp *nova = malloc(sizeof(VariavelComp));
                nova->nome = strdup(vn->var->nome);
                nova->tem_valor = vn->var->tem_valor;
                nova->valor = vn->var->valor ? strdup(vn->var->valor) : NULL;
                nova->prox = NULL;

                // Ajusta valores numéricos
                if (nova->tem_valor && nova->valor && ehNumericoPuro(nova->valor)) {
                    int valorInt = atoi(nova->valor);
                    if (valorInt < 0) {
                        int comp2 = complementoDeDois(valorInt);
                        char buffer[50];
                        sprintf(buffer, "%d", comp2);
                        free(nova->valor);
                        nova->valor = strdup(buffer);
                    }
                }

                if (nova->tem_valor && nova->valor && strchr(nova->valor, '-')) {
                    char novaExpr[512] = "";
                    char copia[512];
                    strcpy(copia, nova->valor);
                
                    char *token = strtok(copia, "+");
                    while (token) {
                        strcat(novaExpr, token);
                        token = strtok(NULL, "+");
                        if (token) strcat(novaExpr, " + ");
                    }
                    free(nova->valor);
                    nova->valor = strdup(novaExpr);
                }

                // Adiciona no final da lista ordenada
                if (!ordenado_inicio) {
                    ordenado_inicio = ordenado_fim = nova;
                } else {
                    ordenado_fim->prox = nova;
                    ordenado_fim = nova;
                }
            }
            vn = vn->prox;
        }
    }

    // Libera memória da lista auxiliar
    while (varNiveis) {
        VarNivel *tmp = varNiveis;
        varNiveis = varNiveis->prox;
        free(tmp);
    }

    return ordenado_inicio;
}

char *processaExpressao(char *exprOriginal) {
    static int contadorVar = 1;  // Contador para geracao do nome da variável
    char novaExpr[512] = "";     // Buffer para a nova expressão
    char token[128]; 
    int i = 0, j = 0;

    while (exprOriginal[i]) {
        // Ignorar espaços
        if (exprOriginal[i] == ' ') {
            novaExpr[strlen(novaExpr)] = exprOriginal[i++];
            continue;
        }

        // Se for operador
        if (exprOriginal[i] == '+' || exprOriginal[i] == '-') {
            novaExpr[strlen(novaExpr)] = exprOriginal[i++];
            continue;
        }

        // Se for número
        if (isdigit(exprOriginal[i])) {
            j = 0;
            while (isdigit(exprOriginal[i])) {
                token[j++] = exprOriginal[i++];
            }
            token[j] = '\0';

            int valor = atoi(token);

            // Verifica se antes tem um '-' (negativo)
            int negativo = 0;
            int k = strlen(novaExpr) - 1;
            while (k >= 0 && novaExpr[k] == ' ') k--;
            if (k >= 0 && novaExpr[k] == '-') {
                negativo = 1;
                novaExpr[k] = '+';  // Troca o '-' por '+' e transforma o número para complemento
            }

            if (negativo) {
                valor = (~valor + 1) & 0xFF;  // Complemento de 2
            }

            sprintf(token, "%d", valor);

            // Cria nome de variável nova
            char varName[32];
            do {
                sprintf(varName, "VAR%d", contadorVar++);
            } while (/* função para checar se VAR<Num> já existe */ ({ 
                VariavelComp *v = variaveis; 
                int existe = 0;
                while (v) {
                    if (strcmp(v->nome, varName) == 0) { existe = 1; break; }
                    v = v->prox;
                }
                existe;
            }));

            // Adicionar nova variável no sistema
            addVar(varName, token);

            // Escrever o nome da variável no lugar do número
            strcat(novaExpr, varName);
        }
        // Se for variável
        else if (isalpha(exprOriginal[i])) {
            j = 0;
            while (isalnum(exprOriginal[i])) {
                token[j++] = exprOriginal[i++];
            }
            token[j] = '\0';
            strcat(novaExpr, token);
        }
        else {
            // Qualquer coisa inválida que passou
            i++;
        }
    }

    return strdup(novaExpr);
}

int expressaoSimplesNumero(char *expr) {
    for (int i = 0; expr[i]; i++) {
        if (!isdigit(expr[i]) && expr[i] != ' ') return 0;
    }
    return 1;
}

void gerarDataCode(FILE *saida) {
    fprintf(saida, ".DATA\n");
    VariavelComp *v = variaveis;
    while (v) {
        printf("Variavel %s -> Valor %s\n", v->nome, v->valor);
        if (v->tem_valor && strspn(v->valor, "0123456789") == strlen(v->valor)) {
            printf("entrou aqui no data\n");
            int valorInt = atoi(v->valor);
            fprintf(saida, "%s DB %02X\n", v->nome, valorInt);
            printf("%s DB %02X\n", v->nome, valorInt);
        }
        else {
            fprintf(saida, "%s DB ?\n", v->nome);
            printf("%s DB ?\n", v->nome);
        }
        v = v->prox;
    }
}

void gerarInstrucaoExpr(FILE *saida, const char *expr, const char *destino) {
    printf("Expressao: %s\n", expr);
    char *copia = strdup(expr);  // Faz uma cópia da expressão
    char *token = strtok(copia, "+-");
    const char *p = expr; 
    int primeiro = 1;

    while (token) {
        // Remover espaços antes da variável (trim)
        while (*token == ' ') token++;
        char *valor = token;

        if (primeiro) {
            primeiro = 0;
            fprintf(saida, "LDA %s\n", valor);
            printf("LDA %s\n", valor);
        } else {
            // Descobre se o operador anterior era + ou -
            while (*p && *p != '+' && *p != '-') p++;  // pula caracteres e espaços
            if (*p == '+') {
                fprintf(saida, "ADD %s\n", valor);
                printf("ADD %s\n", valor);
            } else if (*p == '-') {
                fprintf(saida, "NOT\n");
                fprintf(saida, "ADD %s\n", valor);
                fprintf(saida, "NOT\n");
            }
            p++;  // avança depois do operador
        }
        token = strtok(NULL, "+-");
    }

    fprintf(saida, "STA %s\n", destino);
    free(copia);
}

void gerarCode(FILE *saida) {
    fprintf(saida, "\n.CODE\n.ORG 0\n");
    VariavelComp *v = variaveis;

    while (v) {
        printf("Valor da expressao: %s\n", v->valor);
        if (v->tem_valor && ehExpressao(v->valor)) {
            gerarInstrucaoExpr(saida, v->valor, v->nome);
        } else if (v->tem_valor && strspn(v->valor, "0123456789+-") == strlen(v->valor)) { 
            v = v->prox;
            continue; 
        }
        v = v->prox;
    }

    fprintf(saida, "STA RES\n");
    fprintf(saida, "HLT\n");
}

int validaLinhaPrograma(char *linha) {
    int i = 0;

    // Confere se começa com "PROGRAMA "
    if (strncmp(linha, "PROGRAMA ", 9) != 0) {
        printf("Erro! Declaracao de 'PROGRAMA' nao encontrada!\n");
        return 0;
    }
    i = 9; // pula "PROGRAMA "

    // Confere se o próximo é aspas
    if (linha[i] != '"') {
        return 0;
    }
    i++;

    // Verifica o label
    if (!isalpha(linha[i])) {
        return 0; // Label tem que começar com letra
    }

    while (isalpha(linha[i])) {
        i++;
    }

    // Verifica se fechou aspas
    if (linha[i] != '"') {
        return 0;
    }
    i++;

    // Ignora espaços em branco antes dos dois pontos
    while (linha[i] == ' ') i++;

    // Confere se tem ":"
    if (linha[i] != ':') {
        return 0;
    }

    // Confere se depois do ":" não tem mais nada
    i++;
    if (linha[i] != '\0') {
        return 0;
    }

    return 1; // Tudo OK
}

int leituraCodigo(FILE *entrada) {
    char linha[256];
    int numLinha = 1;
    int inicio = 0;
    int fim = 0;

    rewind(entrada);
    while (fgets(linha, sizeof(linha), entrada)) {
        linha[strcspn(linha, "\r\n")] = 0;

        if(numLinha == 1) {
            if(!validaLinhaPrograma(linha)) {
                printf("Erro: linha %d invalida: %s\n", numLinha, linha);
                return 0;
            }
            else {
                numLinha++;
                continue;
            }
        }

        if(numLinha == 2) {
            if (strstr(linha, "INICIO")) {
                inicio = 1;
                numLinha++;
                continue;
            }
            else {
                printf("Erro: linha %d invalida esperado 'INICIO' recebeu: %s\n", numLinha, linha);
                return 0;
            }
        }

        if(strstr(linha, "FIM")) { 
            printf("Arquivo finalizado!\n");
            fim = 1;
            break; 
        }

        if (linha[0] == '\0') { 
            numLinha++; 
            continue; 
        } 

        char var[32], expr[128];
        if (sscanf(linha, "%s = %[^\n]", var, expr) == 2 && inicio) {
            if (!validaExpressao(expr)) {
                printf("Erro de expressao invalida! Linha %d: %s\n", numLinha, linha);
                return 0;
            }

            printf("Entrou! Linha: %s\n", linha);
            printf("%s = %s\n", var, expr);

            if (expressaoSimplesNumero(expr)) {
                addVar(var, expr); 
            } else {
                char *novaExpr = processaExpressao(expr);
                printf("Expressão processada: %s\n", novaExpr);

                addVar(var, novaExpr); 
                free(novaExpr); 
            }
        }
        else if (inicio) {
            printf("Erro de sintaxe na linha %d: %s\n", numLinha, linha);
            return 0;
        }
        
        numLinha++;
    }

    if(!fim) {
        printf("Erro: Ultima linha invalida esperado 'FIM'\n");
        return 0;
    }

    addVar("RES", NULL); 
    return 1;
}

int execucaoLpnAsm(FILE *entrada) {
    // Cria o arquivo final
    FILE *saida = fopen("arquivos/output.asm", "w");
    if (!saida) {
        perror("Erro ao criar output.asm");
        return 0;
    }
    
    int code = leituraCodigo(entrada); 
    
    if (code) {
        VariavelComp *antigas = variaveis;
        variaveis = ordenarVariaveis();

        // Escreve a seção .DATA
        gerarDataCode(saida);

        // Escreve a seção .CODE
        gerarCode(saida);

        free(antigas);
    } 
    else {
        fclose(saida);
        return 0;
    }

    fclose(saida);

    printf("Compilado com sucesso para output.asm\n");
    return 1;
}