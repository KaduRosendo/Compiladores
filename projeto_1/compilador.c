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
 
    /*  Palavras Reservadas  */
    KW_IF,              /* if */
    KW_ELIF,            /* elif */
    KW_ELSE,            /* else */
    KW_WHILE,           /* while */
    KW_FOR,             /* for */
    KW_BREAK,           /* break */
    KW_CONTINUE,        /* continue */
    KW_RETURN,          /* return */
    KW_DEF,             /* def */
    KW_FROM,            /* from */
    KW_AS,              /* as */
    KW_WITH,            /* with */
    KW_EXEC,            /* exec */
    KW_IN,              /* in */
    KW_IS,              /* is */
    KW_RAISE,           /* raise */
    KW_PRINT,           /* print */
    KW_INPUT,           /* input */
    KW_LEN,             /* len  */
    KW_RANGE,           /* range */
    KW_TRUE,            /* True */
    KW_FALSE,           /* False */
    KW_AND,             /* and */
    KW_OR,              /* or */
    KW_NOT,             /* not */
 
    /* Operadores Aritméticos */
    OP_MAIS,            /* +  */
    OP_MENOS,           /* -  */
    OP_MULT,            /* *  */
    OP_POT,             /* ** */
    OP_DIV,             /* /  */
    OP_MOD,             /* %  */
    OP_TIL,             /* ~  */
 
    /* Operadores Relacionais */
    OP_IGUAL,           /* == */
    OP_DIFERENTE,       /* != */
    OP_DIFERENTE2,      /* <> */
    OP_MENOR,           /* <  */
    OP_MENOR_IGUAL,     /* <= */
    OP_MAIOR,           /* >  */
    OP_MAIOR_IGUAL,     /* >= */
 
    /* Atribuição */
    OP_ATRIB,           /* =  */
 
    /* Delimitadores */
    DEL_ABRE_PAR,       /* (  */
    DEL_FECHA_PAR,      /* )  */
    DEL_ABRE_COL,       /* [  */
    DEL_FECHA_COL,      /* ]  */
    DEL_ABRE_CHAVE,     /* {  */
    DEL_FECHA_CHAVE,    /* }  */
    DEL_VIRGULA,        /* ,  */
    DEL_DOIS_PONTOS,    /* :  */
    DEL_PONTO,          /* .  */
    DEL_PONTO_VIRGULA,  /* ;  */
 
    /* Literais e Identificadores */
    IDENTIFICADOR,      /* nome de variável / função */
    NUMERO,             /* literal inteiro */
    LITERAL_STR         /* literal string  */
 
} TAtomo;


/*Armazena tipo de átomo, linha onde aparece o lexema*/
typedef struct {
    TAtomo atomo;
    int    linha; 
    char   lexema[256];
} TInfoAtomo;


/*TABELA DE NOMES DOS ÁTOMOS*/
static const char *strAtomo[] = {
    "ERRO_LEXICO", "FIM_DE_ARQUIVO",
    "if", "elif", "else", "while", "for", "break", "continue", "return",
    "def", "from", "as", "with", "exec", "in", "is", "raise",
    "print", "input", "len", "range",
    "True", "False",
    "and", "or", "not",
    "+", "-", "*", "**", "/", '%', "~",
    "==", "!=", "<>", "<", "<=", ">", ">=",
    "=",
    "(", ")", "[", "]", "{", "}", ",", ":", ".", ";",
    "IDENTIFICADOR", "NUMERO", "LITERAL_STRING"
};

/*Variáveis Globais do Analisador*/

static char       *entrada       = NULL;
static char       *inicio_buffer = NULL;
static TInfoAtomo lookahead;
static int        contaLinha     = 1;

// Funções do Analisador Léxico
TInfoAtomo obter_atomo(void);
TInfoAtomo reconhece_id_ou_palavra_reservada(void);
TInfoAtomo reconhece_numero(void);
TInfoAtomo reconhece_string(char delim);
TInfoAtomo reconhece_comentario_hash(void);

// Funções do Analisador Sintático
void erro_lexico(const char *msg);
void erro_sintatico(TAtomo esperado);
void erro_geral(const char *msg);
void consome(TAtomo atomo);

void program(void);
void statement_list(void);
void statement(void);
void assignment_or_call(void);
void if_statement(void);
void elif_chain(void);
void else_clause(void);
void while_statement(void);
void for_statement(void);
void def_statement(void);
void return_statement(void);
void print_statement(void);
void break_statement(void);
void continue_statement(void);
void expression(void);
void or_expr(void);
void and_expr(void);
void not_expr(void);
void comparison(void);
void arith_expr(void);
void term(void);
void power(void);
void factor(void);
void atom(void);
void trailer_list(void);
void arglist(void);
void exprlist(void);
void list_literal(void);
int  is_relational(void);
int  is_statement_start(void);
int  is_expr_start(void);


/* 
==================
Função principal 
==================
*/

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_fonte.mp>\n", argv[1]);
        return 1;
    }

    FILE *arquivo = fopen(argv[1], "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo '%s'!\n", argv[1]);
        return 1;
    }

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

    entrada = buffer;
    inicio_buffer = buffer;

    printf("\nIniciando processo de compilacao para MiniPython!\n\n");

    lookahead = obter_atomo();
    program();

    if (lookahead.atomo != FIM_DE_ARQUIVO) {
        erro_geral("Conteudo inesperado apos o fim do programa");
    }
    
    printf("\n-------------------------------------------------------------------\n");
    printf("%d linhas analisadas. Programa lexico e sintaticamente correto!\n",contaLinha);
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

    // Pula espaços, tabs, e novas linhas 
    while (*entrada == ' '  || *entrada == '\t' ||
           *entrada == '\r' || *entrada == '\n') {
        if (*entrada == '\n') contaLinha++;
        entrada++;
    }

    info.linha = contaLinha;

    if (*entrada == '\0') {
        info.atomo - FIM_DE_ARQUIVO;
        return info;
    }

    // Comentário até o final da linha 
    if (*entrada == '#') {
        return reconhece_comentario_hash();
    }
    
    // Palavras Rervadas 
    if (isalpha((unsigned char)*entrada) || *entrada == '_') {
        return reconhece_id_ou_palavra_reservada();
    }

    // Literias numéricos
    if (isdigit((unsigned char)*entrada)) {
        return reconhece_numero();
    }

    // Literais string 
    if (*entrada == '"' || *entrada == '\'') {
        return reconhece_string(*entrada);
    }

    // Operadores e delimitadores
    switch(*entrada) {
        case '+': entrada++; info.atomo = OP_MAIS; return info;
        case '-': entrada++; info.atomo = OP_MENOS; return info;
        case '~': entrada++; info.atomo = OP_TIL; return info;
        case '/': entrada++; info.atomo = OP_DIV; return info;
        case '%': entrada++; info.atomo = OP_MOD; return info;

        case '*':
            entrada++;
            if (*entrada == '*') { entrada++; info.atomo = OP_POT;}
            else                 { info.atomo = OP_MULT;}
            return info;

        case '=':
            entrada++;
            if (*entrada == '=') { entrada++; info.atomo = OP_IGUAL;}
            else                 { info.atomo = OP_ATRIB;}
            return info;

        case '!':
            entrada++;
            if (*entrada == '=') {
                entrada++;
                info.atomo = OP_DIFERENTE;
            } else {
                info.atomo = erro_lexico;
                snprintf(info.lexema, sizeof(info.lexema), "Caractere invalido '!' sem '='");
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

        case '(': entrada++; info.atomo = DEL_ABRE_PAR;    return info;
        case ')': entrada++; info.atomo = DEL_FECHA_PAR;   return info;
        case '[': entrada++; info.atomo = DEL_ABRE_COL;    return info;
        case ']': entrada++; info.atomo = DEL_FECHA_COL;   return info;
        case '{': entrada++; info.atomo = DEL_ABRE_CHAVE;  return info;
        case '}': entrada++; info.atomo = DEL_FECHA_CHAVE; return info;
        case ',': entrada++; info.atomo = DEL_VIRGULA;     return info;
        case ':': entrada++; info.atomo = DEL_DOIS_PONTOS; return info;
        case '.': entrada++; info.atomo = DEL_PONTO;       return info;
        case ';': entrada++; info.atomo = DEL_PONTO_VIRGULA; return info;  

        default:
            info.atomo = ERRO_LEXICO;
            snprintf(info.lexema, sizeof(info.lexema),
                     "Caractere invalido '%c'", *entrada);
            entrada++;
            return info;
    }
    
}

/*
reconhece_comentario_hash()
Consome do # ate o fim da linha 
*/
TInfoAtomo reconhece_comentario_hash(void) {
    while (*entrada != '\n' && *entrada != '\0') {
        entrada++;
    }
    return obter_atomo();
}


/*
reconhece_id_ou_palavra_reservada()
Lê sequência [a-zA-Z0-9_] e verifica se é palavra reservada
*/
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

    /*Mapeamento das palavras reservadas*/
    if      (strcmp(info.lexema, "if") == 0) info.atomo = KW_IF;
    else if (strcmp(info.lexema, "elif") == 0) info.atomo = KW_ELIF;
    else if (strcmp(info.lexema, "else") == 0) info.atomo = KW_ELSE;
    else if (strcmp(info.lexema, "while") == 0) info.atomo = KW_WHILE;
    else if (strcmp(info.lexema, "for") == 0) info.atomo = KW_FOR;
    else if (strcmp(info.lexema, "break") == 0) info.atomo = KW_BREAK;
    else if (strcmp(info.lexema, "continue") == 0) info.atomo = KW_CONTINUE;
    else if (strcmp(info.lexema, "return") == 0) info.atomo = KW_RETURN;
    else if (strcmp(info.lexema, "def") == 0) info.atomo = KW_DEF;
    else if (strcmp(info.lexema, "from") == 0) info.atomo = KW_FROM;
    else if (strcmp(info.lexema, "as") == 0) info.atomo = KW_AS;
    else if (strcmp(info.lexema, "with") == 0) info.atomo = KW_WITH;
    else if (strcmp(info.lexema, "exec") == 0) info.atomo = KW_EXEC;
    else if (strcmp(info.lexema, "in") == 0) info.atomo = KW_IN;
    else if (strcmp(info.lexema, "is") == 0) info.atomo = KW_IS;
    else if (strcmp(info.lexema, "raise") == 0) info.atomo = KW_RAISE;
    else if (strcmp(info.lexema, "print") == 0) info.atomo = KW_PRINT;
    else if (strcmp(info.lexema, "input") == 0) info.atomo = KW_INPUT;
    else if (strcmp(info.lexema, "len") == 0) info.atomo = KW_LEN;
    else if (strcmp(info.lexema, "range") == 0) info.atomo = KW_RANGE;
    else if (strcmp(info.lexema, "True") == 0) info.atomo = KW_TRUE;
    else if (strcmp(info.lexema, "False") == 0) info.atomo = KW_FALSE;
    else if (strcmp(info.lexema, "and") == 0) info.atomo = KW_AND;
    else if (strcmp(info.lexema, "or") == 0) info.atomo = KW_OR;
    else if (strcmp(info.lexema, "not") == 0) info.atomo = KW_NOT;
    else    info.atomo = IDENTIFICADOR;
 
    return info;
}

/*
reconhece_numero()
Lê sequência de dígitos decimais.
*/
TInfoAtomo reconhece_numero(void) {
    TInfoAtomo info;
    memset(&info, 0, sizeof(TInfoAtomo));
    info.linha = contaLinha;
    info.atomo = NUMERO;
 
    char *inicio = entrada;
    while (isdigit((unsigned char)*entrada)) {
        entrada++;
    }
 
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

/*
reconhece_string(delim)
Lê literal string entre aspas simples ou duplas.
*/
TInfoAtomo reconhece_string(char delim) {
    TInfoAtomo info;
    memset(&info, 0, sizeof(TInfoAtomo));
    info.linha = contaLinha;
 
    entrada++; /* Pula o delimitador de abertura */
 
    int idx = 0;
    while (*entrada != '\0' && *entrada != '\n') {
        if (*entrada == delim) {
            entrada++; /* Pula o delimitador de fechamento */
            info.atomo = LITERAL_STR;
            info.lexema[idx] = '\0';
            return info;
        }
        if (*entrada == '\\' && *(entrada + 1) != '\0') {
            entrada++;
            char esc = *entrada;
            char c;
            switch (esc) {
                case 'n':  c = '\n'; break;
                case 't':  c = '\t'; break;
                case '\\': c = '\\'; break;
                case '\'': c = '\''; break;
                case '"':  c = '"';  break;
                default:   c = esc;  break;
            }
            if (idx < (int)sizeof(info.lexema) - 1) info.lexema[idx++] = c;
            entrada++;
        } else {
            if (idx < (int)sizeof(info.lexema) - 1)
                info.lexema[idx++] = *entrada;
            entrada++;
        }
    }
 
    info.atomo = ERRO_LEXICO;
    info.lexema[0] = '\0';
    snprintf(info.lexema, sizeof(info.lexema),
             "String nao fechada na linha %d", info.linha);
    return info;
}

/*
=====================
Analisador Sintático
=====================
*/

void erro_lexico(const char *msg) {
    printf("\n# ERRO LEXICO na linha %d: %s\n", lookahead.linha, msg);
    free(inicio_buffer);
    exit(1);
}

void erro_sintatico(TAtomo esperado) {
    printf("\n# ERRO SINTATICO na linha %d: Esperado '%s'. mas encontrado ''%s", lookahead.linha, strAtomo[esperado], strAtomo[lookahead.atomo]);
    if (lookahead.atomo == IDENTIFICADOR || lookahead.atomo == NUMERO) 
        printf(" ('%s')", lookahead.lexema);
    printf("\n");
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
            lookahead.atomo == NUMERO ||
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
Gramática MiniPython
*/

