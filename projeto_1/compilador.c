/*
Compilador MiniPython - Analisador Léxico e Sintático
Linguagem: MiniPython (subconjunto simplificado do Python)

Integrantes:
Carlos Eduardo Rosendo Basseto - 10409941
Vinicius Oliveira Piccazzio    - 10419471

Para compilar:
gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador

Para executar:
./compilador arquivo.mp

============================================================
EXPRESSÕES REGULARES (conforme especificação do grupo)
============================================================

LETRA              -> [A-Za-z]
DIGITO             -> [0-9]
ID                 -> [A-Za-z_][A-Za-z0-9_]*
NUM                -> [0-9]+
STRING             -> "[^"\n]*"         (somente aspas duplas)
BOOLEANO           -> True | False
COMENTARIO         -> #[^\n]*
OPERADOR_ARITM     -> + | - | ~ | * | ** | / | %
OPERADOR_REL       -> < | > | != | <> | == | <= | >=
OPERADOR_LOGICO    -> and | or | not
OPERADOR_OUT       -> in | is
DELIMITADORES      -> ( | ) | [ | ] | { | } | , | : | . | = | ;
PALAVRAS_RESERVADAS-> return | from | while | as | elif | with | else |
                      if | break | len | input | print | exec | raise |
                      continue | range | def | for

============================================================
GRAMÁTICA LIVRE DE CONTEXTO (conforme especificação do grupo)
============================================================

PROGRAMA          -> LISTA_COMANDOS

LISTA_COMANDOS    -> COMANDO LISTA_COMANDOS | ε

COMANDO           -> ATRIBUICAO | IF | WHILE | FOR | PRINT

ATRIBUICAO        -> ID = EXPRESSAO

-- Expressões --
EXPRESSAO         -> EXPRESSAO_LOGICA
EXPRESSAO_LOGICA  -> EXPRESSAO_REL RESTO_LOGICO
RESTO_LOGICO      -> and EXPRESSAO_REL RESTO_LOGICO
                   | or  EXPRESSAO_REL RESTO_LOGICO
                   | ε
EXPRESSAO_REL     -> EXPRESSAO_ARIT RESTO_REL
RESTO_REL         -> OP_REL EXPRESSAO_ARIT
                   | in   EXPRESSAO_ARIT
                   | is   EXPRESSAO_ARIT
                   | ε
OP_REL            -> < | > | == | != | <= | >= | <>
EXPRESSAO_ARIT    -> TERMO RESTO_ARIT
RESTO_ARIT        -> + TERMO RESTO_ARIT
                   | - TERMO RESTO_ARIT
                   | ε
TERMO             -> FATOR RESTO_TERMO
RESTO_TERMO       -> *  FATOR RESTO_TERMO
                   | /  FATOR RESTO_TERMO
                   | %  FATOR RESTO_TERMO
                   | ** FATOR RESTO_TERMO
                   | ε
FATOR             -> ID | NUM | BOOLEANO | STRING
                   | ( EXPRESSAO )
                   | not FATOR
                   | ~ FATOR

-- Comandos --
IF                -> if EXPRESSAO : COMANDO ELSE_OPC
ELSE_OPC          -> else : COMANDO | ε
WHILE             -> while EXPRESSAO : COMANDO
FOR               -> for ID in range ( EXPRESSAO ) : COMANDO
PRINT             -> print ( LISTA_EXP )
LISTA_EXP         -> EXPRESSAO RESTO_LISTA
RESTO_LISTA       -> , EXPRESSAO RESTO_LISTA | ε
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
=====================
DEFINIÇÕES LÉXICAS
=====================
*/

typedef enum {
    ERRO_LEXICO,
    FIM_DE_ARQUIVO,

    /* Palavras Reservadas */
    KW_IF,              /* if       */
    KW_ELIF,            /* elif     */
    KW_ELSE,            /* else     */
    KW_WHILE,           /* while    */
    KW_FOR,             /* for      */
    KW_BREAK,           /* break    */
    KW_CONTINUE,        /* continue */
    KW_RETURN,          /* return   */
    KW_DEF,             /* def      */
    KW_FROM,            /* from     */
    KW_AS,              /* as       */
    KW_WITH,            /* with     */
    KW_EXEC,            /* exec     */
    KW_RAISE,           /* raise    */
    KW_PRINT,           /* print    */
    KW_INPUT,           /* input    */
    KW_LEN,             /* len      */
    KW_RANGE,           /* range    */

    /* Booleanos */
    KW_TRUE,            /* True     */
    KW_FALSE,           /* False    */

    /* Operadores Lógicos */
    KW_AND,             /* and      */
    KW_OR,              /* or       */
    KW_NOT,             /* not      */

    /* Operadores de Teste */
    KW_IN,              /* in       */
    KW_IS,              /* is       */

    /* Operadores Aritméticos */
    OP_MAIS,            /* +        */
    OP_MENOS,           /* -        */
    OP_TIL,             /* ~        */
    OP_MULT,            /* *        */
    OP_POT,             /* **       */
    OP_DIV,             /* /        */
    OP_MOD,             /* %        */

    /* Operadores Relacionais */
    OP_MENOR,           /* <        */
    OP_MAIOR,           /* >        */
    OP_DIFERENTE,       /* !=       */
    OP_DIFERENTE2,      /* <>       */
    OP_IGUAL,           /* ==       */
    OP_MENOR_IGUAL,     /* <=       */
    OP_MAIOR_IGUAL,     /* >=       */

    /* Atribuição */
    OP_ATRIB,           /* =        */

    /* Delimitadores */
    DEL_ABRE_PAR,       /* (        */
    DEL_FECHA_PAR,      /* )        */
    DEL_ABRE_COL,       /* [        */
    DEL_FECHA_COL,      /* ]        */
    DEL_ABRE_CHAVE,     /* {        */
    DEL_FECHA_CHAVE,    /* }        */
    DEL_VIRGULA,        /* ,        */
    DEL_DOIS_PONTOS,    /* :        */
    DEL_PONTO,          /* .        */
    DEL_PONTO_VIRGULA,  /* ;        */

    /* Literais e Identificadores */
    IDENTIFICADOR,      /* ID  -> [A-Za-z_][A-Za-z0-9_]* */
    NUMERO,             /* NUM -> [0-9]+                  */
    LITERAL_STR         /* STRING -> "[^"\n]*"            */

} TAtomo;

/* Armazena tipo de átomo, linha onde aparece e lexema */
typedef struct {
    TAtomo atomo;
    int    linha;
    char   lexema[256];
    int    id_simbolo;  /* para IDENTIFICADOR: índice na tabela de símbolos */
} TInfoAtomo;

/*
============================================================
TABELA DE SÍMBOLOS
Armazena cada identificador único encontrado no programa.
Cada novo identificador recebe um ID numérico sequencial
(1, 2, 3 ...). Se o mesmo nome aparecer novamente, reutiliza
o mesmo ID — isso garante o comportamento case-sensitive:
'num1', 'Num1' e 'NUM1' são entradas distintas na tabela.
============================================================
*/
#define MAX_SIMBOLOS 1024

typedef struct {
    char nome[256];
} TSimbolo;

static TSimbolo tabela_simbolos[MAX_SIMBOLOS];
static int      num_simbolos = 0;

/*
busca_ou_insere(nome)
Retorna o ID (1-based) do identificador na tabela de símbolos.
Se não existir, insere e retorna o novo ID.
*/
static int busca_ou_insere(const char *nome) {
    int i;
    for (i = 0; i < num_simbolos; i++) {
        if (strcmp(tabela_simbolos[i].nome, nome) == 0)
            return i + 1; // ID 1-based
    }
    if (num_simbolos >= MAX_SIMBOLOS) {
        fprintf(stderr, "Tabela de simbolos cheia!\n");
        exit(1);
    }
    strncpy(tabela_simbolos[num_simbolos].nome, nome, 255);
    tabela_simbolos[num_simbolos].nome[255] = '\0';
    num_simbolos++;
    return num_simbolos; // ID do recém-inserido
}

/* Tabela de nomes dos átomos — mesma ordem da enum TAtomo */
static const char *strAtomo[] = {
    "ERRO_LEXICO", "FIM_DE_ARQUIVO",
    /* Palavras reservadas */
    "if", "elif", "else", "while", "for", "break", "continue", "return",
    "def", "from", "as", "with", "exec", "raise",
    "print", "input", "len", "range",
    /* Booleanos */
    "True", "False",
    /* Operadores lógicos */
    "and", "or", "not",
    /* Operadores de teste */
    "in", "is",
    /* Operadores aritméticos */
    "+", "-", "~", "*", "**", "/", "%",
    /* Operadores relacionais */
    "<", ">", "!=", "<>", "==", "<=", ">=",
    /* Atribuição */
    "=",
    /* Delimitadores */
    "(", ")", "[", "]", "{", "}", ",", ":", ".", ";",
    /* Literais / identificadores */
    "IDENTIFICADOR", "NUMERO", "LITERAL_STRING"
};

/*
============================================================
LISTA DE TOKENS
O analisador léxico roda por completo primeiro, produzindo
uma lista de todos os tokens. Só então o analisador sintático
processa essa lista. Isso garante que todos os erros léxicos
sejam detectados antes de qualquer erro sintático.
============================================================
*/
#define MAX_TOKENS 65536

static TInfoAtomo lista_tokens[MAX_TOKENS];
static int        total_tokens  = 0; // quantos tokens foram gerados
static int        pos_token     = 0; // posição atual na lista (usado pelo sintático)

/* Variáveis globais do analisador léxico */
static FILE *arquivo_fonte = NULL;
static int   char_atual    = 0;   // caractere lido pelo léxico (fgetc)
static int   contaLinha    = 1;

// Funções do Analisador Léxico
static void   proximo_char(void);
static void   pula_espacos(void);
TInfoAtomo    obter_atomo(void);
TInfoAtomo    reconhece_id_ou_palavra_reservada(void);
TInfoAtomo    reconhece_numero(void);
TInfoAtomo    reconhece_string(void);
TInfoAtomo    reconhece_comentario_hash(void);

// Funções do Analisador Sintático
void erro_sintatico(TAtomo esperado);
void erro_geral(const char *msg);
void consome(TAtomo atomo);
TInfoAtomo token_atual(void);

// Regras da gramática
void programa(void);
void lista_comandos(void);
void comando(void);
void atribuicao(void);
void cmd_if(void);
void else_opc(void);
void cmd_while(void);
void cmd_for(void);
void cmd_print(void);
void lista_exp(void);
void resto_lista(void);
void expressao(void);
void expressao_logica(void);
void resto_logico(void);
void expressao_rel(void);
void resto_rel(void);
void expressao_arit(void);
void resto_arit(void);
void termo(void);
void resto_termo(void);
void fator(void);

// Auxiliares
int is_op_rel(void);
int is_inicio_comando(void);
int is_inicio_fator(void);

/*
==================
Função principal
==================
*/

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_fonte.mp>\n", argv[0]);
        return 1;
    }

    arquivo_fonte = fopen(argv[1], "r");
    if (!arquivo_fonte) {
        printf("Erro ao abrir o arquivo '%s'!\n", argv[1]);
        return 1;
    }

    printf("\nIniciando processo de compilacao para MiniPython!\n\n");

    // -------------------------------------------------------
    // FASE 1: ANÁLISE LÉXICA COMPLETA
    // Lê o arquivo caractere por caractere e gera todos os
    // tokens antes de iniciar a análise sintática.
    // -------------------------------------------------------
    printf("[ Fase 1: Analise Lexica ]\n\n");

    proximo_char(); // carrega o primeiro caractere
    while (1) {
        TInfoAtomo tok = obter_atomo();

        if (total_tokens >= MAX_TOKENS) {
            printf("\n# ERRO: Numero maximo de tokens (%d) excedido.\n", MAX_TOKENS);
            fclose(arquivo_fonte);
            return 1;
        }

        // Erro léxico: imprime e encerra imediatamente
        if (tok.atomo == ERRO_LEXICO) {
            printf("\n# ERRO LEXICO na linha %d: %s\n", tok.linha, tok.lexema);
            fclose(arquivo_fonte);
            return 1;
        }

        lista_tokens[total_tokens++] = tok;

        if (tok.atomo == FIM_DE_ARQUIVO) break;
    }

    fclose(arquivo_fonte);

    // Imprime todos os tokens gerados
    for (int i = 0; i < total_tokens; i++) {
        TInfoAtomo *t = &lista_tokens[i];
        if (t->atomo == FIM_DE_ARQUIVO) break;

        // Operadores e palavras reservadas: só linha e nome
        // Identificadores: linha, "IDENTIFICADOR", lexema, id na tabela
        // Números e strings: linha, classe, valor
        // Delimitadores: linha, "DELIMITADOR", símbolo

        switch (t->atomo) {
            case IDENTIFICADOR:
                printf("%d IDENTIFICADOR | %s | id=%d\n",
                       t->linha, t->lexema, t->id_simbolo);
                break;
            case NUMERO:
                printf("%d NUMERO | %s\n", t->linha, t->lexema);
                break;
            case LITERAL_STR:
                printf("%d LITERAL_STRING | %s\n", t->linha, t->lexema);
                break;
            case DEL_ABRE_PAR:
            case DEL_FECHA_PAR:
            case DEL_ABRE_COL:
            case DEL_FECHA_COL:
            case DEL_ABRE_CHAVE:
            case DEL_FECHA_CHAVE:
            case DEL_VIRGULA:
            case DEL_DOIS_PONTOS:
            case DEL_PONTO:
            case DEL_PONTO_VIRGULA:
                printf("%d DELIMITADOR | %s\n", t->linha, strAtomo[t->atomo]);
                break;
            default:
                printf("%d %s\n", t->linha, strAtomo[t->atomo]);
                break;
        }
    }

    // -------------------------------------------------------
    // FASE 2: ANÁLISE SINTÁTICA
    // Percorre a lista de tokens produzida pela fase 1.
    // -------------------------------------------------------
    printf("\n[ Fase 2: Analise Sintatica ]\n\n");

    pos_token = 0;
    programa();

    // Verifica se ainda sobrou algum token nao consumido
    if (token_atual().atomo != FIM_DE_ARQUIVO) {
        TInfoAtomo t = token_atual();

        // Caso especial: numero real (ex: 0.0) - o lexico gera NUM, '.', NUM
        // O '.' sobra apos a atribuicao ser consumida
        if (t.atomo == DEL_PONTO) {
            printf("\n# ERRO SINTATICO na linha %d: Numero real ('.') nao e suportado em MiniPython. "
                   "Apenas inteiros sao permitidos.\n", t.linha);
            return 1;
        }

        // Caso: 'elif' sem 'if' correspondente, ou 'elif' na posicao errada
        if (t.atomo == KW_ELIF) {
            printf("\n# ERRO SINTATICO na linha %d: 'elif' inesperado. "
                   "'elif' so e valido imediatamente apos um bloco 'if', "
                   "e nao faz parte da gramatica como comando independente.\n", t.linha);
            return 1;
        }

        // Caso geral: token que nao pertence a nenhuma regra da gramatica
        printf("\n# ERRO SINTATICO na linha %d: Token '%s'",
               t.linha, strAtomo[t.atomo]);
        if (t.atomo == IDENTIFICADOR || t.atomo == NUMERO)
            printf(" ('%s')", t.lexema);
        printf(" nao e valido neste contexto.\n");
        return 1;
    }

    printf("\n-------------------------------------------------------------------\n");
    printf("%d linhas analisadas. Programa lexico e sintaticamente correto!\n",
           contaLinha);
    printf("-------------------------------------------------------------------\n\n");

    return 0;
}

/*
=================
ANALISADOR LÉXICO
(leitura caractere por caractere via fgetc)
=================
*/

// proximo_char() — avança um caractere no arquivo, contando linhas
static void proximo_char(void) {
    char_atual = fgetc(arquivo_fonte);
    // contaLinha é incrementado quando encontramos '\n' no léxico
}

// pula_espacos() — avança enquanto o caractere atual for espaço/tab/nova linha
static void pula_espacos(void) {
    while (char_atual == ' '  || char_atual == '\t' ||
           char_atual == '\r' || char_atual == '\n') {
        if (char_atual == '\n') contaLinha++;
        proximo_char();
    }
}

/*
obter_atomo()
Reconhece e retorna o próximo token do arquivo fonte,
lendo caractere por caractere com fgetc().
*/
TInfoAtomo obter_atomo(void) {
    TInfoAtomo info;
    memset(&info, 0, sizeof(TInfoAtomo));

    pula_espacos();

    info.linha = contaLinha;

    // Fim de arquivo
    if (char_atual == EOF) {
        info.atomo = FIM_DE_ARQUIVO;
        return info;
    }

    // COMENTARIO -> #[^\n]*
    if (char_atual == '#') {
        return reconhece_comentario_hash();
    }

    // ID -> [A-Za-z_][A-Za-z0-9_]*  e palavras reservadas
    if (isalpha((unsigned char)char_atual) || char_atual == '_') {
        return reconhece_id_ou_palavra_reservada();
    }

    // NUM -> [0-9]+
    if (isdigit((unsigned char)char_atual)) {
        return reconhece_numero();
    }

    // STRING -> "[^"\n]*"  (somente aspas duplas, conforme regex do grupo)
    if (char_atual == '"') {
        return reconhece_string();
    }

    // Operadores e Delimitadores (lidos caractere a caractere)
    switch (char_atual) {

        // Operadores aritméticos
        case '+': proximo_char(); info.atomo = OP_MAIS;  return info;
        case '-': proximo_char(); info.atomo = OP_MENOS; return info;
        case '~': proximo_char(); info.atomo = OP_TIL;   return info;
        case '/': proximo_char(); info.atomo = OP_DIV;   return info;
        case '%': proximo_char(); info.atomo = OP_MOD;   return info;

        case '*':
            proximo_char();
            if (char_atual == '*') { proximo_char(); info.atomo = OP_POT;  }
            else                   {                 info.atomo = OP_MULT; }
            return info;

        // Operadores relacionais e atribuição
        case '=':
            proximo_char();
            if (char_atual == '=') { proximo_char(); info.atomo = OP_IGUAL; }
            else                   {                 info.atomo = OP_ATRIB; }
            return info;

        case '!':
            proximo_char();
            if (char_atual == '=') {
                proximo_char();
                info.atomo = OP_DIFERENTE;
            } else {
                info.atomo = ERRO_LEXICO;
                snprintf(info.lexema, sizeof(info.lexema),
                         "Caractere invalido '!' sem '='");
            }
            return info;

        case '<':
            proximo_char();
            if      (char_atual == '=') { proximo_char(); info.atomo = OP_MENOR_IGUAL; }
            else if (char_atual == '>') { proximo_char(); info.atomo = OP_DIFERENTE2;  }
            else                        {                 info.atomo = OP_MENOR;       }
            return info;

        case '>':
            proximo_char();
            if (char_atual == '=') { proximo_char(); info.atomo = OP_MAIOR_IGUAL; }
            else                   {                 info.atomo = OP_MAIOR;       }
            return info;

        // Delimitadores
        case '(': proximo_char(); info.atomo = DEL_ABRE_PAR;      return info;
        case ')': proximo_char(); info.atomo = DEL_FECHA_PAR;     return info;
        case '[': proximo_char(); info.atomo = DEL_ABRE_COL;      return info;
        case ']': proximo_char(); info.atomo = DEL_FECHA_COL;     return info;
        case '{': proximo_char(); info.atomo = DEL_ABRE_CHAVE;    return info;
        case '}': proximo_char(); info.atomo = DEL_FECHA_CHAVE;   return info;
        case ',': proximo_char(); info.atomo = DEL_VIRGULA;       return info;
        case ':': proximo_char(); info.atomo = DEL_DOIS_PONTOS;   return info;
        case '.': proximo_char(); info.atomo = DEL_PONTO;         return info;
        case ';': proximo_char(); info.atomo = DEL_PONTO_VIRGULA; return info;

        default:
            info.atomo = ERRO_LEXICO;
            snprintf(info.lexema, sizeof(info.lexema),
                     "Caractere invalido '%c'", (char)char_atual);
            proximo_char();
            return info;
    }
}

// reconhece_comentario_hash() — COMENTARIO -> #[^\n]*
TInfoAtomo reconhece_comentario_hash(void) {
    // Consome do '#' até antes do '\n' ou EOF
    while (char_atual != '\n' && char_atual != EOF) {
        proximo_char();
    }
    return obter_atomo(); // retorna o próximo token real
}

/*
reconhece_id_ou_palavra_reservada()
Lê [A-Za-z_][A-Za-z0-9_]* caractere a caractere.
Verifica se é palavra reservada ou identificador.
Identificadores são registrados na tabela de símbolos.
*/
TInfoAtomo reconhece_id_ou_palavra_reservada(void) {
    TInfoAtomo info;
    memset(&info, 0, sizeof(TInfoAtomo));
    info.linha = contaLinha;

    int idx = 0;
    // Consome [A-Za-z0-9_]* caractere a caractere
    while (isalnum((unsigned char)char_atual) || char_atual == '_') {
        if (idx < (int)sizeof(info.lexema) - 1)
            info.lexema[idx++] = (char)char_atual;
        proximo_char();
    }
    info.lexema[idx] = '\0';

    // Mapeamento das palavras reservadas (case-sensitive)
    if      (strcmp(info.lexema, "if")       == 0) info.atomo = KW_IF;
    else if (strcmp(info.lexema, "elif")     == 0) info.atomo = KW_ELIF;
    else if (strcmp(info.lexema, "else")     == 0) info.atomo = KW_ELSE;
    else if (strcmp(info.lexema, "while")    == 0) info.atomo = KW_WHILE;
    else if (strcmp(info.lexema, "for")      == 0) info.atomo = KW_FOR;
    else if (strcmp(info.lexema, "break")    == 0) info.atomo = KW_BREAK;
    else if (strcmp(info.lexema, "continue") == 0) info.atomo = KW_CONTINUE;
    else if (strcmp(info.lexema, "return")   == 0) info.atomo = KW_RETURN;
    else if (strcmp(info.lexema, "def")      == 0) info.atomo = KW_DEF;
    else if (strcmp(info.lexema, "from")     == 0) info.atomo = KW_FROM;
    else if (strcmp(info.lexema, "as")       == 0) info.atomo = KW_AS;
    else if (strcmp(info.lexema, "with")     == 0) info.atomo = KW_WITH;
    else if (strcmp(info.lexema, "exec")     == 0) info.atomo = KW_EXEC;
    else if (strcmp(info.lexema, "raise")    == 0) info.atomo = KW_RAISE;
    else if (strcmp(info.lexema, "print")    == 0) info.atomo = KW_PRINT;
    else if (strcmp(info.lexema, "input")    == 0) info.atomo = KW_INPUT;
    else if (strcmp(info.lexema, "len")      == 0) info.atomo = KW_LEN;
    else if (strcmp(info.lexema, "range")    == 0) info.atomo = KW_RANGE;
    else if (strcmp(info.lexema, "True")     == 0) info.atomo = KW_TRUE;
    else if (strcmp(info.lexema, "False")    == 0) info.atomo = KW_FALSE;
    else if (strcmp(info.lexema, "and")      == 0) info.atomo = KW_AND;
    else if (strcmp(info.lexema, "or")       == 0) info.atomo = KW_OR;
    else if (strcmp(info.lexema, "not")      == 0) info.atomo = KW_NOT;
    else if (strcmp(info.lexema, "in")       == 0) info.atomo = KW_IN;
    else if (strcmp(info.lexema, "is")       == 0) info.atomo = KW_IS;
    else {
        // Identificador: registra na tabela de símbolos
        info.atomo      = IDENTIFICADOR;
        info.id_simbolo = busca_ou_insere(info.lexema);
    }

    return info;
}

/*
reconhece_numero()
Lê [0-9]+ caractere a caractere.
Erro léxico se seguido de letra ou '_' (ex: 12Val).
*/
TInfoAtomo reconhece_numero(void) {
    TInfoAtomo info;
    memset(&info, 0, sizeof(TInfoAtomo));
    info.linha = contaLinha;
    info.atomo = NUMERO;

    int idx = 0;
    // Consome dígitos: [0-9]+
    while (isdigit((unsigned char)char_atual)) {
        if (idx < (int)sizeof(info.lexema) - 1)
            info.lexema[idx++] = (char)char_atual;
        proximo_char();
    }
    info.lexema[idx] = '\0';

    // Erro lexico: numero real (ex: 0.0) nao e suportado — so inteiros
    if (char_atual == '.') {
        info.atomo = ERRO_LEXICO;
        // Consome o '.' e os digitos seguintes para montar o lexema completo
        char tmp[256];
        strncpy(tmp, info.lexema, idx);
        int t = idx;
        tmp[t++] = '.';
        proximo_char(); // consome o '.'
        while (isdigit((unsigned char)char_atual)) {
            if (t < 200) tmp[t++] = (char)char_atual;
            proximo_char();
        }
        tmp[t] = '\0';
        snprintf(info.lexema, 220,
                 "Numero real '%.150s' nao e suportado. Apenas inteiros sao permitidos.", tmp);
        return info;
    }

    // Erro lexico: token invalido como "12Val"
    if (isalpha((unsigned char)char_atual) || char_atual == '_') {
        info.atomo = ERRO_LEXICO;
        char tmp[256];
        strncpy(tmp, info.lexema, idx);
        tmp[idx] = '\0';
        int t = idx;
        while (isalnum((unsigned char)char_atual) || char_atual == '_') {
            if (t < 100) tmp[t++] = (char)char_atual;
            proximo_char();
        }
        tmp[t] = '\0';
        snprintf(info.lexema, 220, "ID invalido '%.200s'", tmp);
    }

    return info;
}

/*
reconhece_string()
Lê "[^"\n]*" caractere a caractere.
Somente aspas duplas (conforme regex do grupo).
Erro léxico se não fechar na mesma linha.
*/
TInfoAtomo reconhece_string(void) {
    TInfoAtomo info;
    memset(&info, 0, sizeof(TInfoAtomo));
    info.linha = contaLinha;

    proximo_char(); // Consome a aspas dupla de abertura '"'

    int idx = 0;
    // Lê [^"\n]* caractere a caractere
    while (char_atual != EOF && char_atual != '\n' && char_atual != '"') {
        if (idx < (int)sizeof(info.lexema) - 1)
            info.lexema[idx++] = (char)char_atual;
        proximo_char();
    }

    if (char_atual == '"') {
        proximo_char(); // Consome a aspas dupla de fechamento
        info.atomo = LITERAL_STR;
        info.lexema[idx] = '\0';
        return info;
    }

    // String não fechada antes do fim de linha ou do arquivo
    info.atomo = ERRO_LEXICO;
    snprintf(info.lexema, sizeof(info.lexema),
             "String nao fechada na linha %d", info.linha);
    return info;
}

/*
=====================
ANALISADOR SINTÁTICO
(trabalha sobre lista_tokens[] produzida pelo léxico)
=====================
*/

// token_atual() — retorna o token na posição corrente da lista
TInfoAtomo token_atual(void) {
    if (pos_token < total_tokens)
        return lista_tokens[pos_token];
    return lista_tokens[total_tokens - 1]; // último é sempre FIM_DE_ARQUIVO
}

/*
erro_sintatico(esperado)
Informa qual token era esperado e qual foi encontrado.
Quando o token encontrado e FIM_DE_ARQUIVO, usa a linha do
ultimo token consumido — assim o erro aponta para a linha
onde o simbolo faltante deveria ter aparecido.
*/
void erro_sintatico(TAtomo esperado) {
    TInfoAtomo t = token_atual();

    // Determina a linha correta do erro:
    // Se o token inesperado esta em linha posterior ao ultimo token consumido
    // (ou e FIM_DE_ARQUIVO), reporta a linha do ultimo token consumido,
    // pois e la que o simbolo esperado deveria ter aparecido.
    int linha_erro = t.linha;
    if (pos_token > 0) {
        int linha_anterior = lista_tokens[pos_token - 1].linha;
        if (t.atomo == FIM_DE_ARQUIVO || t.linha > linha_anterior) {
            linha_erro = linha_anterior;
        }
    }

    printf("\n# ERRO SINTATICO na linha %d: Esperado '%s', mas encontrado '%s'",
           linha_erro, strAtomo[esperado], strAtomo[t.atomo]);
    if (t.atomo == IDENTIFICADOR || t.atomo == NUMERO)
        printf(" ('%s')", t.lexema);
    printf("\n");
    exit(1);
}

void erro_geral(const char *msg) {
    TInfoAtomo t = token_atual();
    printf("\n# ERRO na linha %d: %s\n", t.linha, msg);
    exit(1);
}

/*
consome(atomo)
Verifica se o token atual é o esperado.
Se sim: imprime no formato correto e avança para o próximo token.
Se não: chama erro_sintatico() com o token esperado.
*/
void consome(TAtomo atomo) {
    TInfoAtomo t = token_atual();

    if (t.atomo == atomo) {
        // Imprime conforme a classe do token
        switch (t.atomo) {
            case IDENTIFICADOR:
                printf("%d IDENTIFICADOR | %s | id=%d\n",
                       t.linha, t.lexema, t.id_simbolo);
                break;
            case NUMERO:
                printf("%d NUMERO | %s\n", t.linha, t.lexema);
                break;
            case LITERAL_STR:
                printf("%d LITERAL_STRING | %s\n", t.linha, t.lexema);
                break;
            case DEL_ABRE_PAR:
            case DEL_FECHA_PAR:
            case DEL_ABRE_COL:
            case DEL_FECHA_COL:
            case DEL_ABRE_CHAVE:
            case DEL_FECHA_CHAVE:
            case DEL_VIRGULA:
            case DEL_DOIS_PONTOS:
            case DEL_PONTO:
            case DEL_PONTO_VIRGULA:
                printf("%d DELIMITADOR | %s\n", t.linha, strAtomo[t.atomo]);
                break;
            default:
                printf("%d %s\n", t.linha, strAtomo[t.atomo]);
                break;
        }
        pos_token++;
    } else {
        erro_sintatico(atomo);
    }
}

/*
===================
GRAMÁTICA MINIPYTHON
===================
*/

// is_inicio_fator() — retorna 1 se o token atual pode iniciar um FATOR
int is_inicio_fator(void) {
    switch (token_atual().atomo) {
        case IDENTIFICADOR:
        case NUMERO:
        case LITERAL_STR:
        case KW_TRUE:
        case KW_FALSE:
        case DEL_ABRE_PAR:
        case DEL_ABRE_COL:
        case KW_NOT:
        case OP_TIL:
        case KW_INPUT:
        case KW_LEN:
            return 1;
        default:
            return 0;
    }
}

// is_inicio_comando() — retorna 1 se o token atual pode iniciar um COMANDO
int is_inicio_comando(void) {
    switch (token_atual().atomo) {
        case IDENTIFICADOR:
        case KW_IF:
        case KW_WHILE:
        case KW_FOR:
        case KW_PRINT:
            return 1;
        default:
            return 0;
    }
}

// is_op_rel() — retorna 1 se o token atual é um operador relacional
int is_op_rel(void) {
    switch (token_atual().atomo) {
        case OP_MENOR:       case OP_MAIOR:
        case OP_DIFERENTE:   case OP_DIFERENTE2:
        case OP_IGUAL:       case OP_MENOR_IGUAL:
        case OP_MAIOR_IGUAL: case KW_IN:
        case KW_IS:
            return 1;
        default:
            return 0;
    }
}

// PROGRAMA -> LISTA_COMANDOS
void programa(void) {
    lista_comandos();
}

// LISTA_COMANDOS -> COMANDO LISTA_COMANDOS | ε
void lista_comandos(void) {
    while (is_inicio_comando()) {
        comando();
    }
}

// COMANDO -> ATRIBUICAO | IF | WHILE | FOR | PRINT
void comando(void) {
    switch (token_atual().atomo) {
        case IDENTIFICADOR: atribuicao(); break;
        case KW_IF:         cmd_if();     break;
        case KW_WHILE:      cmd_while();  break;
        case KW_FOR:        cmd_for();    break;
        case KW_PRINT:      cmd_print();  break;
        default:
            erro_geral("Comando invalido. Esperado: atribuicao, if, while, for ou print.");
    }
}

// ATRIBUICAO -> ID = EXPRESSAO
void atribuicao(void) {
    consome(IDENTIFICADOR);
    consome(OP_ATRIB);
    expressao();
}

// IF -> if EXPRESSAO : COMANDO ELSE_OPC
void cmd_if(void) {
    consome(KW_IF);
    expressao();
    consome(DEL_DOIS_PONTOS);
    comando();
    else_opc();
}

// ELSE_OPC -> else : COMANDO | ε
void else_opc(void) {
    if (token_atual().atomo == KW_ELSE) {
        consome(KW_ELSE);
        consome(DEL_DOIS_PONTOS);
        comando();
    }
}

// WHILE -> while EXPRESSAO : COMANDO
void cmd_while(void) {
    consome(KW_WHILE);
    expressao();
    consome(DEL_DOIS_PONTOS);
    comando();
}

// FOR -> for ID in range ( EXPRESSAO ) : COMANDO
void cmd_for(void) {
    consome(KW_FOR);
    consome(IDENTIFICADOR);
    consome(KW_IN);
    consome(KW_RANGE);
    consome(DEL_ABRE_PAR);
    expressao();
    consome(DEL_FECHA_PAR);
    consome(DEL_DOIS_PONTOS);
    comando();
}

// PRINT -> print ( LISTA_EXP )
void cmd_print(void) {
    consome(KW_PRINT);
    consome(DEL_ABRE_PAR);
    lista_exp();
    consome(DEL_FECHA_PAR);
}

// LISTA_EXP -> EXPRESSAO RESTO_LISTA
void lista_exp(void) {
    expressao();
    resto_lista();
}

// RESTO_LISTA -> , EXPRESSAO RESTO_LISTA | ε
void resto_lista(void) {
    if (token_atual().atomo == DEL_VIRGULA) {
        consome(DEL_VIRGULA);
        expressao();
        resto_lista();
    }
}

/*
======================
GRAMÁTICA DE EXPRESSÕES
======================
Hierarquia de precedência (menor -> maior):

  EXPRESSAO        = EXPRESSAO_LOGICA
  EXPRESSAO_LOGICA = EXPRESSAO_REL RESTO_LOGICO
  RESTO_LOGICO     = and ... | or ... | ε
  EXPRESSAO_REL    = EXPRESSAO_ARIT RESTO_REL
  RESTO_REL        = OP_REL ... | in ... | is ... | ε
  EXPRESSAO_ARIT   = TERMO RESTO_ARIT
  RESTO_ARIT       = + ... | - ... | ε
  TERMO            = FATOR RESTO_TERMO
  RESTO_TERMO      = * | / | % | ** | ε
  FATOR            = ID | NUM | BOOL | STR | (expr) | not F | ~ F
*/

// EXPRESSAO -> EXPRESSAO_LOGICA
void expressao(void) {
    expressao_logica();
}

// EXPRESSAO_LOGICA -> EXPRESSAO_REL RESTO_LOGICO
void expressao_logica(void) {
    expressao_rel();
    resto_logico();
}

// RESTO_LOGICO -> and EXPRESSAO_REL RESTO_LOGICO | or EXPRESSAO_REL RESTO_LOGICO | ε
void resto_logico(void) {
    if (token_atual().atomo == KW_AND) {
        consome(KW_AND);
        expressao_rel();
        resto_logico();
    } else if (token_atual().atomo == KW_OR) {
        consome(KW_OR);
        expressao_rel();
        resto_logico();
    }
    // ε: nenhuma ação
}

// EXPRESSAO_REL -> EXPRESSAO_ARIT RESTO_REL
void expressao_rel(void) {
    expressao_arit();
    resto_rel();
}

// RESTO_REL -> OP_REL EXPRESSAO_ARIT | in EXPRESSAO_ARIT | is EXPRESSAO_ARIT | ε
void resto_rel(void) {
    if (is_op_rel()) {
        consome(token_atual().atomo);
        expressao_arit();
    }
    // ε: nenhuma ação
}

// EXPRESSAO_ARIT -> TERMO RESTO_ARIT
void expressao_arit(void) {
    termo();
    resto_arit();
}

// RESTO_ARIT -> + TERMO RESTO_ARIT | - TERMO RESTO_ARIT | ε
void resto_arit(void) {
    if (token_atual().atomo == OP_MAIS) {
        consome(OP_MAIS);
        termo();
        resto_arit();
    } else if (token_atual().atomo == OP_MENOS) {
        consome(OP_MENOS);
        termo();
        resto_arit();
    }
    // ε: nenhuma ação
}

// TERMO -> FATOR RESTO_TERMO
void termo(void) {
    fator();
    resto_termo();
}

// RESTO_TERMO -> ** | * | / | % FATOR RESTO_TERMO | ε
void resto_termo(void) {
    if (token_atual().atomo == OP_POT) {
        consome(OP_POT);
        fator();
        resto_termo();
    } else if (token_atual().atomo == OP_MULT) {
        consome(OP_MULT);
        fator();
        resto_termo();
    } else if (token_atual().atomo == OP_DIV) {
        consome(OP_DIV);
        fator();
        resto_termo();
    } else if (token_atual().atomo == OP_MOD) {
        consome(OP_MOD);
        fator();
        resto_termo();
    }
    // ε: nenhuma ação
}

/*
FATOR -> ID | NUM | BOOLEANO | STRING
       | ( EXPRESSAO )
       | not FATOR
       | ~ FATOR

Extensões práticas:
  - input(expr) como fator: x = input("msg")
  - len(expr)   como fator: while i < len(v):
  - ID[expr]    acesso a lista: print(v[i])
  - ID(args)    chamada de função: resultado = func(x)
  - [expr, ...] lista literal: v = [1, 2, 3]
*/
void fator(void) {
    switch (token_atual().atomo) {

        case IDENTIFICADOR:
            consome(IDENTIFICADOR);
            // Trailer: chamada de função ID(args)
            if (token_atual().atomo == DEL_ABRE_PAR) {
                consome(DEL_ABRE_PAR);
                if (is_inicio_fator()) lista_exp();
                consome(DEL_FECHA_PAR);
            }
            // Trailer: acesso a lista ID[expr]
            if (token_atual().atomo == DEL_ABRE_COL) {
                consome(DEL_ABRE_COL);
                expressao();
                consome(DEL_FECHA_COL);
            }
            break;

        case NUMERO:
            consome(NUMERO);
            break;

        // BOOLEANO -> True | False
        case KW_TRUE:
            consome(KW_TRUE);
            break;

        case KW_FALSE:
            consome(KW_FALSE);
            break;

        // STRING -> "[^"\n]*"
        case LITERAL_STR:
            consome(LITERAL_STR);
            break;

        // ( EXPRESSAO )
        case DEL_ABRE_PAR:
            consome(DEL_ABRE_PAR);
            expressao();
            consome(DEL_FECHA_PAR);
            break;

        // not FATOR
        case KW_NOT:
            consome(KW_NOT);
            fator();
            break;

        // ~ FATOR
        case OP_TIL:
            consome(OP_TIL);
            fator();
            break;

        // input(expr) como fator: x = input("mensagem")
        case KW_INPUT:
            consome(KW_INPUT);
            consome(DEL_ABRE_PAR);
            if (is_inicio_fator()) expressao();
            consome(DEL_FECHA_PAR);
            break;

        // len(expr) como fator: while i < len(v):
        case KW_LEN:
            consome(KW_LEN);
            consome(DEL_ABRE_PAR);
            expressao();
            consome(DEL_FECHA_PAR);
            break;

        // Lista literal: [expr, expr, ...]  ou  []
        case DEL_ABRE_COL:
            consome(DEL_ABRE_COL);
            if (is_inicio_fator()) lista_exp();
            consome(DEL_FECHA_COL);
            break;

        default:
            erro_geral("Expressao invalida: fator inesperado.");
    }
}
