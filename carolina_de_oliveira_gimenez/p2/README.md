# Projeto P2 — Gramática BNF do Cinnamocode

Este diretório contém a especificação da gramática BNF para a linguagem "Cinnamocode", utilizada como base para o desenvolvimento do compilador na etapa P2.

## 📁 Arquivo principal

- `bnf-cinnamocode.txt`: Descreve formalmente a sintaxe da linguagem, incluindo declarações de variáveis, funções, comandos de controle de fluxo, expressões aritméticas e booleanas.

## 🔤 Estrutura da Gramática

A gramática cobre:

- Declarações de variáveis e funções
- Comandos (atribuição, if, while, return, chamada de função)
- Expressões aritméticas e booleanas, com precedência e associatividade
- Tipos primitivos: `int`, `float`, `bool`
- Identificadores, números inteiros e decimais, booleanos (`true`/`false`)

## ℹ️ Observações

- O símbolo `E` representa a produção vazia (epsilon).
- Os operadores lógicos e relacionais estão representados por palavras-chave (`AND`, `OR`, `EQUAL`, `DIFF`, etc).

Consulte o arquivo `bnf-cinnamocode.txt` para detalhes completos da sintaxe.

## 🐧 Desenvolvedora

- Carolina de Oliveira Gimenez