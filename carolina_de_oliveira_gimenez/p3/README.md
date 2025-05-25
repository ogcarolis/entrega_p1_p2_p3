# Projeto P3 — Brainfuck

Este projeto contém um compilador e um interpretador para a linguagem Brainfuck, implementado em C.

## ⏺️ Como compilar

No terminal, dentro desta pasta, execute:

```
gcc -Wall -Werror bfe.c -o bfe
gcc -Wall -Werror ./utils/arvore.c bfc.c -o bfc
```

## ▶️ Como executar

Rode um programa Brainfuck passando a entrada via stdin, por exemplo:

```
echo "CRÉDITO=5-12*5+3" | ./bfc | ./bfe
```

## 🗂️ Estrutura

- `bfe.c`: Interpretador Brainfuck.
- `bfc.h`: Header do Compilador Brainfuck.
- `bfc.c`: Compilador Brainfuck.
- `utils/`: Utilitários e possíveis extensões para análise de expressões.

## ℹ️ Observações

- O interpretador utiliza uma fita de 30.000 células, conforme o padrão Brainfuck.
- O ambiente de testes é Linux.

## 🐧 Desenvolvedora

- Carolina de Oliveira Gimenez
