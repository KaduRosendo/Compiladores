# Projeto - Compiladores: Analisador Léxico e Sintático (MiniPython)

**Integrantes:**
* Carlos Eduardo Rosendo Basseto - 10409941
* Vinicius Oliveira Piccazzio - 10419471

## 1. Parte do Trabalho Concluída
O projeto foi concluído atendendo às especificações das Etapas 1, 2 e 3. O analisador léxico e o sintático funcionam de forma integrada. O programa lê um código-fonte via linha de comando, gera os tokens e realiza a análise sintática validando a estrutura do código com base na gramática definida.

## 2. Como Executar o Código

Para compilar o código fonte usando o GCC, abra o terminal e execute:
```bash
gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador
```
## 3. Decisões de Design e Implementação

- Gerenciamento de Memória: Todo o conteúdo do arquivo ```.mp``` é carregado para um buffer na memória de uma só vez no início da execução. Isso melhora a performance e facilita a manipulação do ponteiro de leitura no analisador léxico.

- Estrutura de Tokens: Os átomos são definidos em um ```enum TAtomo``` e passados entre o léxico e o sintático através de uma struct que guarda o tipo de token, o lexema (string original) e a linha em que ocorreu.

- Parser Descendente Recursivo: O analisador sintático foi construído mapeando cada regra de produção da gramática para uma função específica no C (ex: ```comando()```, ```expressao()```). A sincronização e avanço de tokens ocorrem por meio da função central ```consome()```.

## 4. Bugs ou Erros Identificados

Não foram identificados bugs de execução (como segmentation fault) durante os testes. O compilador trata erros léxicos (ex: aspas não fechadas, caracteres não reconhecidos) e sintáticos (falta de dois pontos, estrutura fora de ordem), interrompendo a execução de forma controlada e exibindo a linha e o motivo da falha.

## 5. Expressões Regulares

LETRA               -> [A-Za-z]

DIGITO              -> [0-9]

ID                  -> [A-Za-z_][A-Za-z0-9_]*

NUM                 -> [0-9]+

STRING              -> "[^"\n]*"

BOOLEANO            -> True | False

COMENTARIO          -> #[^\n]*

OPERADOR_ARITMETICO -> + | - | ~ | * | ** | / | %

OPERADOR_RELACIONAL -> < | > | != | <> | == | <= | >=

OPERADOR_LOGICO     -> and | or | not

OPERADOR_OUT        -> in | is

DELIMITADORES       -> ( | ) | [ | ] | { | } | , | : | . | = | ;

PALAVRAS_RESERVADAS -> return | from | while | as | elif | with | else | if | break | len | input | print | exec | raise | continue | range | def | for

## 6. Gramática Livre de Contexto

PROGRAMA         -> LISTA_COMANDOS

LISTA_COMANDOS   -> COMANDO LISTA_COMANDOS | ε

COMANDO          -> ATRIBUICAO | IF | WHILE | FOR | PRINT

ATRIBUICAO       -> ID = EXPRESSAO


-- Expressões --


EXPRESSAO        -> EXPRESSAO_LOGICA

EXPRESSAO_LOGICA -> EXPRESSAO_REL RESTO_LOGICO

RESTO_LOGICO     -> and EXPRESSAO_REL RESTO_LOGICO | or EXPRESSAO_REL RESTO_LOGICO | ε

EXPRESSAO_REL    -> EXPRESSAO_ARIT RESTO_REL

RESTO_REL        -> OP_REL EXPRESSAO_ARIT | in EXPRESSAO_ARIT | is EXPRESSAO_ARIT | ε

OP_REL           -> < | > | == | != | <= | >= | <>

EXPRESSAO_ARIT   -> TERMO RESTO_ARIT

RESTO_ARIT       -> + TERMO RESTO_ARIT | - TERMO RESTO_ARIT | ε

TERMO            -> FATOR RESTO_TERMO

RESTO_TERMO      -> * FATOR RESTO_TERMO | / FATOR RESTO_TERMO | % FATOR RESTO_TERMO | ** FATOR RESTO_TERMO | ε

FATOR            -> ID | NUM | BOOLEANO | STRING | ( EXPRESSAO ) | not FATOR | ~ FATOR

-- Comandos Estruturais --

IF               -> if EXPRESSAO : COMANDO ELSE_OPC

ELSE_OPC         -> else : COMANDO | ε

WHILE            -> while EXPRESSAO : COMANDO

FOR              -> for ID in range ( EXPRESSAO ) : COMANDO

PRINT            -> print ( LISTA_EXP )

LISTA_EXP        -> EXPRESSAO RESTO_LISTA

RESTO_LISTA      -> , EXPRESSAO RESTO_LISTA | ε
