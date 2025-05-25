## Carolina de Oliveira Gimenez

## ☑️ Observações

A estrutura do programa deve seguir este padrão:

- **SEMPRE** na linha 1 deve conter a declaração do programa sendo de tal forma: 
`PROGRAMA "<label>":`

Onde 
	
 	1. A palavra `PROGRAMA` deve estar sempre em maiúsculo seguida de um espaço somente.
	2. Após o espaço deve-se abrir aspas duplas.
	3. Em seguida deve conter uma `label` que é composta somente de letras maiúsculas ou minúsculas.
	4. Após o fim da label, deve-se fechar as aspas.
	5. Por fim, é colocado `:`, não podendo haver nada após esse caractere.
	
- Em seguida, **SEMPRE** na linha 2 deve conter a palavra `INICIO` e seguir para a próxima linha.
- Após isso, é feito o corpo do programa, onde ele apenas aceita expressões de tal forma:
`<variavel> = <variavel> | <expressao>`

Onde:

	1. Apenas são aceitas expressões de soma e subtração
	2. O corpo **SEMPRE** deve conter uma variavel recebendo a expressão
	3. Não é possível colocar uma expressão sem variável

- No final do código **SEMPRE** deve haver uma linha com a **SOMENTE** a palavra `FIM`

### Exemplo de programa
```
PROGRAMA "teste":
INICIO
a = 5

b = 10 + 10 + 10 - 19

c = a - b
FIM
```

### Compilação e Execução do código

A compilação do código deve ser feita desta forma:

`gcc -Wall -Werror assembler.c executor.c compilador.c main.c -o compilador`

E a execução do código deve ser feita desta forma:
`./compilador`
