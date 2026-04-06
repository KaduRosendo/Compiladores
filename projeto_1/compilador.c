/*
Compilador MiniPython - Analisador Léxico e Sintático
Linguagem: MiniPython 

Integrantes:
Carlos Eduardo Rosendo Basseto - 10409941
Vinicius Oliveira Piccazzio    - 10419471

Para Compilar:
gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador

Para Executar:
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
} TInfoAtomo;

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

/* Variáveis globais do analisador */
static char      *entrada       = NULL;
static char      *inicio_buffer = NULL;
static TInfoAtomo lookahead;
static int        contaLinha    = 1;

// Funções do Analisador Léxico
TInfoAtomo obter_atomo(void);
TInfoAtomo reconhece_id_ou_palavra_reservada(void);
TInfoAtomo reconhece_numero(void);
TInfoAtomo reconhece_string(void);
TInfoAtomo reconhece_comentario_hash(void);

// Funções do Analisador Sintático
void erro_lexico(const char *msg);
void erro_sintatico(TAtomo esperado);
void erro_geral(const char *msg);
void consome(TAtomo atomo);

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

    FILE *arquivo = fopen(argv[1], "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo '%s'!\n", argv[1]);
        return 1;
    }

    // Lê o arquivo inteiro para um buffer em memória
    fseek(arquivo, 0, SEEK_END);
    long tamanho = ftell(arquivo);
    fseek(arquivo, 0, SEEK_SET);

    char *buffer = (char *)malloc(tamanho + 1);
    if (!buffer) {
        printf("Erro de alocacao de memoria!\n");
        fclose(arquivo);
        return 1;
    }

    size_t lidos = fread(buffer, 1, tamanho, arquivo);
    buffer[lidos] = '\0';
    fclose(arquivo);

    entrada       = buffer;
    inicio_buffer = buffer;

    printf("\nIniciando processo de compilacao para MiniPython!\n\n");

    // Obtém o primeiro token e inicia a análise sintática
    lookahead = obter_atomo();
    programa();

    if (lookahead.atomo != FIM_DE_ARQUIVO) {
        erro_geral("Conteudo inesperado apos o fim do programa.");
    }

    printf("\n-------------------------------------------------------------------\n");
    printf("%d linhas analisadas. Programa lexico e sintaticamente correto!\n",
           contaLinha);
    printf("-------------------------------------------------------------------\n\n");

    free(inicio_buffer);
    return 0;
}

/*
=================
ANALISADOR LÉXICO
=================
*/

TInfoAtomo obter_atomo(void) {
    TInfoAtomo info;
    memset(&info, 0, sizeof(TInfoAtomo));

    // Pula espaços, tabs e novas linhas
    while (*entrada == ' '  || *entrada == '\t' ||
           *entrada == '\r' || *entrada == '\n') {
        if (*entrada == '\n') contaLinha++;
        entrada++;
    }

    info.linha = contaLinha;

    // Fim de arquivo
    if (*entrada == '\0') {
        info.atomo = FIM_DE_ARQUIVO;
        return info;
    }

    // COMENTARIO -> #[^\n]*  (ignora até fim da linha)
    if (*entrada == '#') {
        return reconhece_comentario_hash();
    }

    // ID -> [A-Za-z_][A-Za-z0-9_]*  e palavras reservadas
    if (isalpha((unsigned char)*entrada) || *entrada == '_') {
        return reconhece_id_ou_palavra_reservada();
    }

    // NUM -> [0-9]+
    if (isdigit((unsigned char)*entrada)) {
        return reconhece_numero();
    }

    // STRING -> "[^"\n]*"  (somente aspas duplas, conforme regex do grupo)
    if (*entrada == '"') {
        return reconhece_string();
    }

    // Operadores e delimitadores
    switch (*entrada) {

        // Operadores aritméticos
        case '+': entrada++; info.atomo = OP_MAIS;  return info;
        case '-': entrada++; info.atomo = OP_MENOS; return info;
        case '~': entrada++; info.atomo = OP_TIL;   return info;
        case '/': entrada++; info.atomo = OP_DIV;   return info;
        case '%': entrada++; info.atomo = OP_MOD;   return info;

        case '*':
            entrada++;
            if (*entrada == '*') { entrada++; info.atomo = OP_POT;  }
            else                 {            info.atomo = OP_MULT; }
            return info;

        // Operadores relacionais e atribuição
        case '=':
            entrada++;
            if (*entrada == '=') { entrada++; info.atomo = OP_IGUAL;  }
            else                 {            info.atomo = OP_ATRIB;  }
            return info;

        case '!':
            entrada++;
            if (*entrada == '=') {
                entrada++;
                info.atomo = OP_DIFERENTE;
            } else {
                info.atomo = ERRO_LEXICO;
                snprintf(info.lexema, sizeof(info.lexema),
                         "Caractere invalido '!' sem '='");
            }
            return info;

        case '<':
            entrada++;
            if      (*entrada == '=') { entrada++; info.atomo = OP_MENOR_IGUAL; }
            else if (*entrada == '>') { entrada++; info.atomo = OP_DIFERENTE2;  }
            else                     {             info.atomo = OP_MENOR;       }
            return info;

        case '>':
            entrada++;
            if (*entrada == '=') { entrada++; info.atomo = OP_MAIOR_IGUAL; }
            else                 {            info.atomo = OP_MAIOR;       }
            return info;

        // Delimitadores
        case '(': entrada++; info.atomo = DEL_ABRE_PAR;      return info;
        case ')': entrada++; info.atomo = DEL_FECHA_PAR;     return info;
        case '[': entrada++; info.atomo = DEL_ABRE_COL;      return info;
        case ']': entrada++; info.atomo = DEL_FECHA_COL;     return info;
        case '{': entrada++; info.atomo = DEL_ABRE_CHAVE;    return info;
        case '}': entrada++; info.atomo = DEL_FECHA_CHAVE;   return info;
        case ',': entrada++; info.atomo = DEL_VIRGULA;       return info;
        case ':': entrada++; info.atomo = DEL_DOIS_PONTOS;   return info;
        case '.': entrada++; info.atomo = DEL_PONTO;         return info;
        case ';': entrada++; info.atomo = DEL_PONTO_VIRGULA; return info;

        default:
            info.atomo = ERRO_LEXICO;
            snprintf(info.lexema, sizeof(info.lexema),
                     "Caractere invalido '%c'", *entrada);
            entrada++;
            return info;
    }
}

// reconhece_comentario_hash() — COMENTARIO -> #[^\n]*
TInfoAtomo reconhece_comentario_hash(void) {
    while (*entrada != '\n' && *entrada != '\0') {
        entrada++;
    }
    return obter_atomo();
}

// reconhece_id_ou_palavra_reservada() — ID -> [A-Za-z_][A-Za-z0-9_]*
TInfoAtomo reconhece_id_ou_palavra_reservada(void) {
    TInfoAtomo info;
    memset(&info, 0, sizeof(TInfoAtomo));
    info.linha = contaLinha;

    char *inicio = entrada;
    while (isalnum((unsigned char)*entrada) || *entrada == '_') {
        entrada++;
    }

    int tam = (int)(entrada - inicio);
    if (tam >= (int)sizeof(info.lexema)) tam = (int)sizeof(info.lexema) - 1;
    strncpy(info.lexema, inicio, tam);
    info.lexema[tam] = '\0';

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
    else    info.atomo = IDENTIFICADOR;

    return info;
}

// reconhece_numero() — NUM -> [0-9]+
TInfoAtomo reconhece_numero(void) {
    TInfoAtomo info;
    memset(&info, 0, sizeof(TInfoAtomo));
    info.linha = contaLinha;
    info.atomo = NUMERO;

    char *inicio = entrada;
    while (isdigit((unsigned char)*entrada)) {
        entrada++;
    }

    // Erro léxico: token inválido como "12Val"
    if (isalpha((unsigned char)*entrada) || *entrada == '_') {
        info.atomo = ERRO_LEXICO;
        while (isalnum((unsigned char)*entrada) || *entrada == '_') {
            entrada++;
        }
        int tam = (int)(entrada - inicio);
        if (tam > 100) tam = 100;
        char tmp[128];
        strncpy(tmp, inicio, tam);
        tmp[tam] = '\0';
        snprintf(info.lexema, sizeof(info.lexema), "ID invalido '%s'", tmp);
        return info;
    }

    int tam = (int)(entrada - inicio);
    if (tam >= (int)sizeof(info.lexema)) tam = (int)sizeof(info.lexema) - 1;
    strncpy(info.lexema, inicio, tam);
    info.lexema[tam] = '\0';
    return info;
}

// reconhece_string() — STRING -> "[^"\n]*"  (somente aspas duplas)
TInfoAtomo reconhece_string(void) {
    TInfoAtomo info;
    memset(&info, 0, sizeof(TInfoAtomo));
    info.linha = contaLinha;

    entrada++; // Pula a aspas dupla de abertura

    int idx = 0;
    // Consome [^"\n]* — qualquer char exceto '"' e '\n'
    while (*entrada != '\0' && *entrada != '\n' && *entrada != '"') {
        if (idx < (int)sizeof(info.lexema) - 1)
            info.lexema[idx++] = *entrada;
        entrada++;
    }

    if (*entrada == '"') {
        entrada++; // Pula a aspas dupla de fechamento
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
=====================
*/

void erro_lexico(const char *msg) {
    printf("\n# ERRO LEXICO na linha %d: %s\n", lookahead.linha, msg);
    free(inicio_buffer);
    exit(1);
}

void erro_sintatico(TAtomo esperado) {
    printf("\n# ERRO SINTATICO na linha %d: Esperado '%s', mas encontrado '%s'",
           lookahead.linha, strAtomo[esperado], strAtomo[lookahead.atomo]);
    if (lookahead.atomo == IDENTIFICADOR || lookahead.atomo == NUMERO)
        printf(" ('%s')", lookahead.lexema);
    printf("\n");
    free(inicio_buffer);
    exit(1);
}

void erro_geral(const char *msg) {
    printf("\n# ERRO na linha %d: %s\n", lookahead.linha, msg);
    free(inicio_buffer);
    exit(1);
}

/*
consome(atomo)
Verifica o lookahead, imprime e avança para o próximo token
*/
void consome(TAtomo atomo) {
    if (lookahead.atomo == ERRO_LEXICO) {
        erro_lexico(lookahead.lexema);
    }
    if (lookahead.atomo == atomo) {
        printf("%d %s", lookahead.linha, strAtomo[lookahead.atomo]);
        if (lookahead.atomo == IDENTIFICADOR ||
            lookahead.atomo == NUMERO        ||
            lookahead.atomo == LITERAL_STR) {
            printf(" | %s", lookahead.lexema);
        }
        printf("\n");
        lookahead = obter_atomo();
    } else {
        erro_sintatico(atomo);
    }
}

/*
===================
GRAMÁTICA MINIPYTHON
===================
*/

// is_inicio_fator() — retorna 1 se o lookahead pode iniciar um FATOR
int is_inicio_fator(void) {
    switch (lookahead.atomo) {
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

// is_inicio_comando() — retorna 1 se o lookahead pode iniciar um COMANDO
int is_inicio_comando(void) {
    switch (lookahead.atomo) {
        case IDENTIFICADOR: // ATRIBUICAO -> ID = EXPRESSAO
        case KW_IF:
        case KW_WHILE:
        case KW_FOR:
        case KW_PRINT:
            return 1;
        default:
            return 0;
    }
}

// is_op_rel() — retorna 1 se o lookahead é um operador relacional
int is_op_rel(void) {
    switch (lookahead.atomo) {
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
    if (lookahead.atomo == ERRO_LEXICO) erro_lexico(lookahead.lexema);

    while (is_inicio_comando()) {
        comando();
        if (lookahead.atomo == ERRO_LEXICO) erro_lexico(lookahead.lexema);
    }

    if (lookahead.atomo == ERRO_LEXICO) erro_lexico(lookahead.lexema);
}

// COMANDO -> ATRIBUICAO | IF | WHILE | FOR | PRINT
void comando(void) {
    switch (lookahead.atomo) {
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
    if (lookahead.atomo == KW_ELSE) {
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
    if (lookahead.atomo == DEL_VIRGULA) {
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
    if (lookahead.atomo == KW_AND) {
        consome(KW_AND);
        expressao_rel();
        resto_logico();
    } else if (lookahead.atomo == KW_OR) {
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
        consome(lookahead.atomo);
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
    if (lookahead.atomo == OP_MAIS) {
        consome(OP_MAIS);
        termo();
        resto_arit();
    } else if (lookahead.atomo == OP_MENOS) {
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

// RESTO_TERMO -> * FATOR RESTO_TERMO | / FATOR RESTO_TERMO | % FATOR RESTO_TERMO | ** FATOR RESTO_TERMO | ε
void resto_termo(void) {
    if (lookahead.atomo == OP_POT) {
        consome(OP_POT);
        fator();
        resto_termo();
    } else if (lookahead.atomo == OP_MULT) {
        consome(OP_MULT);
        fator();
        resto_termo();
    } else if (lookahead.atomo == OP_DIV) {
        consome(OP_DIV);
        fator();
        resto_termo();
    } else if (lookahead.atomo == OP_MOD) {
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
    // Intercepta erro léxico pendente antes do switch
    if (lookahead.atomo == ERRO_LEXICO) erro_lexico(lookahead.lexema);

    switch (lookahead.atomo) {

        case IDENTIFICADOR:
            consome(IDENTIFICADOR);
            // Trailer: chamada de função ID(args)
            if (lookahead.atomo == DEL_ABRE_PAR) {
                consome(DEL_ABRE_PAR);
                if (is_inicio_fator()) lista_exp();
                consome(DEL_FECHA_PAR);
            }
            // Trailer: acesso a lista ID[expr]
            if (lookahead.atomo == DEL_ABRE_COL) {
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
