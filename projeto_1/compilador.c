/*
Integrantes:
Carlos Eduardo Rosendo Basseto - 10409941
João Rocha Murgel - 10410293


/*
Para compilar no vscode use:
gcc compilador.c -Wall -Og -g -o compilador
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

// ---------- DEFINIÇÕES LÉXICAS (MINIPASCAL) ----------

// Enumeração de tokens para a linguagem MiniPascal 
typedef enum {
    ERROLEXICO, FIMDEARQUIVO,

    // Palavras Reservadas
    PROGRAM, VAR, INTEGER, BOOLEAN, PROCEDURE, BEGIN, END, IF, THEN, ELSE, WHILE, DO,
    WRITE, READ, TRUE, FALSE,

    // Símbolos e Operadores
    PONTO_VIRGULA, DOIS_PONTOS, PONTO, VIRGULA,
    ATRIBUICAO, // :=
    MAIS, MENOS, MULT,
    DIV, // Palavra reservada para divisão
    E_LOGICO, OU_LOGICO, NAO_LOGICO, // and, or, not
    IGUAL, DIFERENTE, MENOR, MENOR_IGUAL, MAIOR, MAIOR_IGUAL,

    // Outros
    IDENTIFICADOR, NUMERO,
    ABRE_PARENTESES, FECHA_PARENTESES
} TAtomo;

// Estrutura para armazenar informações do token
typedef struct {
    TAtomo atomo;
    int linha;
    char lexema[30]; // Armazena o texto do token (ID, número, etc)
} TInfoAtomo;

// Variáveis Globais
char *entrada = NULL;
TInfoAtomo lookahead;
int contaLinha = 1;

// Mapeamento de TAtomo para String para facilitar a depuração e erros
char *strAtomo[] = {
    "ERROLEXICO", "FIMDEARQUIVO",
    "PROGRAM", "VAR", "INTEGER", "BOOLEAN", "PROCEDURE", "BEGIN", "END", "IF", "THEN", "ELSE", "WHILE", "DO",
    "WRITE", "READ", "TRUE", "FALSE",
    ";", ":", ".", ",",
    ":=",
    "+", "-", "*",
    "div",
    "and", "or", "not",
    "=", "<>", "<", "<=", ">", ">=",
    "IDENTIFICADOR", "NUMERO",
    "(", ")"
};

// ---------- DECLARAÇÃO DAS FUNÇÕES ----------

// Funções do Analisador Léxico
TInfoAtomo obter_atomo();
TInfoAtomo reconhece_id_ou_palavra_reservada();
TInfoAtomo reconhece_numero();
TInfoAtomo reconhece_comentario();

// Funções do Analisador Sintático (baseado na gramática MiniPascal)
void consome(TAtomo atomo);
void program();
void block();
void variable_declaration_part();
void variable_declaration();
void type();
void statement_part();
void statement();
void assignment_statement();
void if_statement();
void while_statement();
void write_statement();
void expression();
void simple_expression();
void term();
void factor();
TAtomo relational_operator();


int main(int num_argumentos, char **argumentos) {
    if (num_argumentos < 2) {
        printf("Uso: %s <arquivo_fonte.pas>\n", argumentos[0]);
        return 1;
    }

    FILE *arquivo = fopen(argumentos[1], "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo %s!\n", argumentos[1]);
        return 1;
    }

    fseek(arquivo, 0, SEEK_END);
    long tamanho = ftell(arquivo);
    fseek(arquivo, 0, SEEK_SET);

    char *buffer = (char *)malloc(tamanho + 1);
    if (buffer == NULL) {
        printf("Erro de alocação de memória!\n");
        fclose(arquivo);
        return 1;
    }

    size_t lidos = fread(buffer, 1, tamanho, arquivo);
    buffer[lidos] = '\0';
    fclose(arquivo);

    entrada = buffer;
    printf("\nIniciando processo de compilacao para MiniPascal!\n\n");

    lookahead = obter_atomo();

    program(); // Ponto de entrada da análise sintática

    if (lookahead.atomo != FIMDEARQUIVO) {
        printf("\n# ERRO SINTATICO na linha %d: Esperado fim de arquivo, mas encontrado '%s'\n", lookahead.linha, strAtomo[lookahead.atomo]);
        free(buffer);
        exit(1);
    }

    printf("\n•••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••\n");
    printf("%d linhas analisadas. Programa lexica e sintaticamente correto!\n", contaLinha);
    printf("•••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••\n\n");

    free(buffer);
    return 0;
}

// ---------- ANALISADOR LÉXICO ----------

TInfoAtomo obter_atomo() {
    TInfoAtomo info_atomo;
    memset(&info_atomo, 0, sizeof(TInfoAtomo));

    // Ignora espaços, tabs e novas linhas
    while (isspace(*entrada)) {
        if (*entrada == '\n') contaLinha++;
        entrada++;
    }

    info_atomo.linha = contaLinha;

    // Fim de arquivo
    if (*entrada == '\0') {
        info_atomo.atomo = FIMDEARQUIVO;
        return info_atomo;
    }

    // Identificadores e Palavras Reservadas
    if (isalpha(*entrada) || *entrada == '_') {
        return reconhece_id_ou_palavra_reservada();
    }

    // Números
    if (isdigit(*entrada)) {
        return reconhece_numero();
    }

    // Comentários e operador de atribuição
    if (*entrada == '/' && *(entrada + 1) == '*') {
        return reconhece_comentario();
    }

    // Operadores e delimitadores
    switch (*entrada) {
        case '+': entrada++; info_atomo.atomo = MAIS; return info_atomo;
        case '-': entrada++; info_atomo.atomo = MENOS; return info_atomo;
        case '*': entrada++; info_atomo.atomo = MULT; return info_atomo;
        case '=': entrada++; info_atomo.atomo = IGUAL; return info_atomo;
        case '(': entrada++; info_atomo.atomo = ABRE_PARENTESES; return info_atomo;
        case ')': entrada++; info_atomo.atomo = FECHA_PARENTESES; return info_atomo;
        case ';': entrada++; info_atomo.atomo = PONTO_VIRGULA; return info_atomo;
        case ',': entrada++; info_atomo.atomo = VIRGULA; return info_atomo;
        case '.': entrada++; info_atomo.atomo = PONTO; return info_atomo;

        case ':':
            entrada++;
            if (*entrada == '=') {
                entrada++;
                info_atomo.atomo = ATRIBUICAO;
            } else {
                info_atomo.atomo = DOIS_PONTOS;
            }
            return info_atomo;
        
        case '<':
            entrada++;
            if (*entrada == '>') {
                entrada++;
                info_atomo.atomo = DIFERENTE;
            } else if (*entrada == '=') {
                entrada++;
                info_atomo.atomo = MENOR_IGUAL;
            } else {
                info_atomo.atomo = MENOR;
            }
            return info_atomo;

        case '>':
            entrada++;
            if (*entrada == '=') {
                entrada++;
                info_atomo.atomo = MAIOR_IGUAL;
            } else {
                info_atomo.atomo = MAIOR;
            }
            return info_atomo;

        default:
            info_atomo.atomo = ERROLEXICO;
            info_atomo.lexema[0] = *entrada;
            entrada++;
            return info_atomo;
    }
}

TInfoAtomo reconhece_id_ou_palavra_reservada() {
    TInfoAtomo info_atomo;
    info_atomo.linha = contaLinha;
    int tamanho = 0;

    while (isalnum(*entrada) || *entrada == '_') {
        if (tamanho < 29) {
            info_atomo.lexema[tamanho++] = *entrada;
        }
        entrada++;
    }
    info_atomo.lexema[tamanho] = '\0';

    // Checar se é palavra reservada
    if (strcmp(info_atomo.lexema, "program") == 0) info_atomo.atomo = PROGRAM;
    else if (strcmp(info_atomo.lexema, "var") == 0) info_atomo.atomo = VAR;
    else if (strcmp(info_atomo.lexema, "integer") == 0) info_atomo.atomo = INTEGER;
    else if (strcmp(info_atomo.lexema, "boolean") == 0) info_atomo.atomo = BOOLEAN;
    else if (strcmp(info_atomo.lexema, "procedure") == 0) info_atomo.atomo = PROCEDURE;
    else if (strcmp(info_atomo.lexema, "begin") == 0) info_atomo.atomo = BEGIN;
    else if (strcmp(info_atomo.lexema, "end") == 0) info_atomo.atomo = END;
    else if (strcmp(info_atomo.lexema, "if") == 0) info_atomo.atomo = IF;
    else if (strcmp(info_atomo.lexema, "then") == 0) info_atomo.atomo = THEN;
    else if (strcmp(info_atomo.lexema, "else") == 0) info_atomo.atomo = ELSE;
    else if (strcmp(info_atomo.lexema, "while") == 0) info_atomo.atomo = WHILE;
    else if (strcmp(info_atomo.lexema, "do") == 0) info_atomo.atomo = DO;
    else if (strcmp(info_atomo.lexema, "write") == 0) info_atomo.atomo = WRITE;
    else if (strcmp(info_atomo.lexema, "read") == 0) info_atomo.atomo = READ;
    else if (strcmp(info_atomo.lexema, "true") == 0) info_atomo.atomo = TRUE;
    else if (strcmp(info_atomo.lexema, "false") == 0) info_atomo.atomo = FALSE;
    else if (strcmp(info_atomo.lexema, "div") == 0) info_atomo.atomo = DIV;
    else if (strcmp(info_atomo.lexema, "and") == 0) info_atomo.atomo = E_LOGICO;
    else if (strcmp(info_atomo.lexema, "or") == 0) info_atomo.atomo = OU_LOGICO;
    else if (strcmp(info_atomo.lexema, "not") == 0) info_atomo.atomo = NAO_LOGICO;
    else info_atomo.atomo = IDENTIFICADOR; // Se não for, é um identificador

    return info_atomo;
}

TInfoAtomo reconhece_numero() {
    TInfoAtomo info_atomo;
    info_atomo.linha = contaLinha;
    info_atomo.atomo = NUMERO;
    int tamanho = 0;

    while (isdigit(*entrada)) {
        if (tamanho < 29) {
            info_atomo.lexema[tamanho++] = *entrada;
        }
        entrada++;
    }
    info_atomo.lexema[tamanho] = '\0';
    return info_atomo;
}

TInfoAtomo reconhece_comentario() {
    entrada += 2; // Pula o "/*"
    while (!(*entrada == '*' && *(entrada + 1) == '/')) {
        if (*entrada == '\0') {
            TInfoAtomo erro;
            erro.atomo = ERROLEXICO;
            erro.linha = contaLinha;
            strcpy(erro.lexema, "Comentario nao fechado");
            return erro;
        }
        if (*entrada == '\n') contaLinha++;
        entrada++;
    }
    entrada += 2; // Pula o "*/"
    return obter_atomo(); // Retorna o próximo token válido depois do comentário
}

// ---------- ANALISADOR SINTÁTICO ----------

void consome(TAtomo atomo) {
    if (lookahead.atomo == atomo) {
        printf("Consumido: Linha %d, Token %s", lookahead.linha, strAtomo[lookahead.atomo]);
        if(lookahead.atomo == IDENTIFICADOR || lookahead.atomo == NUMERO){
            printf(" (%s)", lookahead.lexema);
        }
        printf("\n");
        lookahead = obter_atomo();
    } else {
        printf("\n# ERRO SINTATICO na linha %d: Esperado '%s', mas encontrado '%s'\n", lookahead.linha, strAtomo[atomo], strAtomo[lookahead.atomo]);
        free(entrada - strlen(entrada)); // Libera o buffer original
        exit(1);
    }
}

// <program> ::= program <identifier>; <block>.
void program() {
    consome(PROGRAM);
    consome(IDENTIFICADOR);
    consome(PONTO_VIRGULA);
    block();
    consome(PONTO);
}

// <block> ::= <variable_declaration_part> <statement_part>
void block() {
    variable_declaration_part();
    statement_part();
}

// <variable_declaration_part> ::= var <variable_declaration>; {<variable_declaration>;} | <empty>
void variable_declaration_part() {
    if (lookahead.atomo == VAR) {
        consome(VAR);
        // Loop para múltiplas linhas de declaração de variáveis
        while (lookahead.atomo == IDENTIFICADOR) {
            variable_declaration();
            consome(PONTO_VIRGULA);
        }
    }
}

// <variable_declaration> ::= <identifier> {, <identifier>} : <type>
void variable_declaration() {
    consome(IDENTIFICADOR);
    while (lookahead.atomo == VIRGULA) {
        consome(VIRGULA);
        consome(IDENTIFICADOR);
    }
    consome(DOIS_PONTOS);
    type();
}

// <type> ::= integer | boolean
void type() {
    if (lookahead.atomo == INTEGER) {
        consome(INTEGER);
    } else if (lookahead.atomo == BOOLEAN) {
        consome(BOOLEAN);
    } else {
       printf("\n# ERRO SINTATICO na linha %d: Esperado um tipo (integer ou boolean), mas encontrado '%s'\n", lookahead.linha, strAtomo[lookahead.atomo]);
       exit(1);
    }
}

// <statement_part> ::= begin <statement> {; <statement>} end 
void statement_part() {
    consome(BEGIN);
    statement();
    while (lookahead.atomo == PONTO_VIRGULA) {
        consome(PONTO_VIRGULA);
        statement();
    }
    consome(END);
}

// <statement> ::= <assignment_statement> | <if_statement> | <while_statement> | ...
void statement() {
    switch (lookahead.atomo) {
        case IDENTIFICADOR:
            assignment_statement();
            break;
        case IF:
            if_statement();
            break;
        case WHILE:
            while_statement();
            break;
        case WRITE:
            write_statement();
            break;
        case BEGIN:
            statement_part(); // Bloco aninhado
            break;
        default:
            // Pode ser um statement vazio se seguido por 'end'
            if (lookahead.atomo != END) {
                printf("\n# ERRO SINTATICO na linha %d: Comando invalido iniciado com '%s'\n", lookahead.linha, strAtomo[lookahead.atomo]);
                exit(1);
            }
    }
}

// <assignment_statement> ::= <variable> := <expression> 
void assignment_statement() {
    consome(IDENTIFICADOR);
    consome(ATRIBUICAO);
    expression();
}

// <if_statement> ::= if <expression> then <statement> [else <statement>] 
void if_statement() {
    consome(IF);
    expression();
    consome(THEN);
    statement();
    if (lookahead.atomo == ELSE) {
        consome(ELSE);
        statement();
    }
}

// <while_statement> ::= while <expression> do <statement> 
void while_statement() {
    consome(WHILE);
    expression();
    consome(DO);
    statement();
}

// <write_statement> ::= write ( <variable> ) (Simplificado para uma variável)
void write_statement() {
    consome(WRITE);
    consome(ABRE_PARENTESES);
    expression(); // write pode imprimir o resultado de uma expressão
    consome(FECHA_PARENTESES);
}

// <expression> ::= <simple_expression> [ <relational_operator> <simple_expression> ] 
void expression() {
    simple_expression();
    TAtomo op = relational_operator();
    if (op != FIMDEARQUIVO) { // Se for um operador relacional válido
        consome(op);
        simple_expression();
    }
}

// <simple_expression> ::= <term> { <adding_operator> <term> } 
void simple_expression() {
    term();
    while (lookahead.atomo == MAIS || lookahead.atomo == MENOS || lookahead.atomo == OU_LOGICO) {
        consome(lookahead.atomo);
        term();
    }
}

// <term> ::= <factor> { <multiplying_operator> <factor> } 
void term() {
    factor();
    while (lookahead.atomo == MULT || lookahead.atomo == DIV || lookahead.atomo == E_LOGICO) {
        consome(lookahead.atomo);
        factor();
    }
}

// <factor> ::= <variable> | <constant> | ( <expression> ) | not <factor> 
void factor() {
    switch (lookahead.atomo) {
        case IDENTIFICADOR:
        case NUMERO:
        case TRUE:
        case FALSE:
            consome(lookahead.atomo);
            break;
        case ABRE_PARENTESES:
            consome(ABRE_PARENTESES);
            expression();
            consome(FECHA_PARENTESES);
            break;
        case NAO_LOGICO:
            consome(NAO_LOGICO);
            factor();
            break;
        default:
            printf("\n# ERRO SINTATICO na linha %d: Fator invalido na expressao, encontrado '%s'\n", lookahead.linha, strAtomo[lookahead.atomo]);
            exit(1);
    }
}

// Retorna o token do operador relacional se existir, senão retorna FIMDEARQUIVO
TAtomo relational_operator() {
    switch(lookahead.atomo) {
        case IGUAL:
        case DIFERENTE:
        case MENOR:
        case MENOR_IGUAL:
        case MAIOR:
        case MAIOR_IGUAL:
            return lookahead.atomo;
        default:
            return FIMDEARQUIVO;
    }
}
