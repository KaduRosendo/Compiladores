/*
Integrantes:
Carlos Eduardo Rosendo Basseto - 10409941
João Rocha Murgel - 10410293
*/

/*
Para compilar no vscode use:
gcc compilador.c -Wall -Og -g -o compilador
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

// Definições Léxicas
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
char *inicio_buffer = NULL;
TInfoAtomo lookahead;
int contaLinha = 1;

// Mapeamento de TAtomo para String para facilitar a depuração e erros
char *strAtomo[] = {
    "ERROLEXICO", "FIMDEARQUIVO", "PROGRAM", "VAR", "INTEGER", "BOOLEAN", "PROCEDURE", "BEGIN", "END", "IF", "THEN", "ELSE", "WHILE", "DO",
    "WRITE", "READ", "TRUE", "FALSE", ";", ":", ".", ",", ":=", "+", "-", "*", "div", "and", "or", "not",
    "=", "<>", "<", "<=", ">", ">=", "IDENTIFICADOR", "NUMERO", "(", ")"
};

// Funções do Analisador Léxico
TInfoAtomo obter_atomo();
TInfoAtomo reconhece_id_ou_palavra_reservada();
TInfoAtomo reconhece_numero();
TInfoAtomo reconhece_comentario_barra_asterisco();
TInfoAtomo reconhece_comentario_chaves();

// Funções do Analisador Sintático
void erro_sintatico(TAtomo esperado);
void erro_geral(const char* mensagem);
void erro_lexico();
void consome(TAtomo atomo);
void program();
void block();
void variable_declaration_part();
void variable_declaration();
void procedure_declaration_part();
void parameter_list();
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
        printf("Uso: %s <arquivo_fonte.mp>\n", argumentos[0]);
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
    inicio_buffer = buffer;
    printf("\nIniciando processo de compilacao para MiniPascal!\n\n");
    lookahead = obter_atomo();

    if (lookahead.atomo == ERROLEXICO) {
        erro_lexico();
    }
    
    program(); // Ponto de entrada da análise sintática

    if (lookahead.atomo != FIMDEARQUIVO) {
        erro_geral("Conteudo inesperado apos o '.' final do programa.");
    }

    printf("\n-------------------------------------------------------------------\n");
    printf("%d linhas analisadas. Programa lexica e sintaticamente correto!\n", contaLinha);
    printf("-------------------------------------------------------------------\n\n");

    free(inicio_buffer);
    return 0;
}

// Analisador Léxico

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
    // Comentários
    if (*entrada == '/' && *(entrada + 1) == '*') {
        return reconhece_comentario_barra_asterisco();
    }
    if (*entrada == '{') {
        return reconhece_comentario_chaves();
    }
    // Identificadores e Palavras Reservadas
    if (isalpha(*entrada) || *entrada == '_') {
        return reconhece_id_ou_palavra_reservada();
    }
    // Números
    if (isdigit(*entrada)) {
        return reconhece_numero();
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
            info_atomo.lexema[1] = '\0';
            entrada++;
            return info_atomo;
    }
}

TInfoAtomo reconhece_id_ou_palavra_reservada() {
    TInfoAtomo info_atomo;
    info_atomo.linha = contaLinha;
    int tamanho = 0;
    char* inicio_lexema = entrada;
    while (isalnum(*entrada) || *entrada == '_') {
        entrada++;
    }
    tamanho = entrada - inicio_lexema;
    if (tamanho > 29) tamanho = 29;
    strncpy(info_atomo.lexema, inicio_lexema, tamanho);
    info_atomo.lexema[tamanho] = '\0';

    // Palavras Reservadas
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
    char* inicio_lexema = entrada;
    while (isdigit(*entrada)) {
        entrada++;
    }
    if (isalpha(*entrada) || *entrada == '_') {
        info_atomo.atomo = ERROLEXICO;
        while(isalnum(*entrada) || *entrada == '_') {
            entrada++;
        }
        int tamanho_total = entrada - inicio_lexema;
        if (tamanho_total > 29) tamanho_total = 29;
        char token_invalido[30];
        strncpy(token_invalido, inicio_lexema, tamanho_total);
        token_invalido[tamanho_total] = '\0';
        strncpy(info_atomo.lexema, token_invalido, 29);
        info_atomo.lexema[29] = '\0';
        return info_atomo;
    }
    int tamanho = entrada - inicio_lexema;
    if (tamanho > 29) tamanho = 29;
    strncpy(info_atomo.lexema, inicio_lexema, tamanho);
    info_atomo.lexema[tamanho] = '\0';
    return info_atomo;
}

TInfoAtomo reconhece_comentario_barra_asterisco() {
    int linha_inicio_comentario = contaLinha;
    entrada += 2; // Pula o "/*"
    while (!(*entrada == '*' && *(entrada + 1) == '/')) {
        if (*entrada == '\0') {
            TInfoAtomo erro;
            erro.atomo = ERROLEXICO;
            erro.linha = linha_inicio_comentario;
            strncpy(erro.lexema, "Comentario '/*' nao fechado", 29);
            erro.lexema[29] = '\0';
            return erro;
        }
        if (*entrada == '\n') contaLinha++;
        entrada++;
    }
    entrada += 2; // Pula o "*/"
    return obter_atomo(); // Retorna o próximo token válido
}

TInfoAtomo reconhece_comentario_chaves() {
    int linha_inicio_comentario = contaLinha;
    entrada++; // Pula o "{"
    while (*entrada != '}') {
        if (*entrada == '\0') {
            TInfoAtomo erro;
            erro.atomo = ERROLEXICO;
            erro.linha = linha_inicio_comentario;
            strncpy(erro.lexema, "Comentario '{' nao fechado", 29);
            erro.lexema[29] = '\0';
            return erro;
        }
        if (*entrada == '\n') contaLinha++;
        entrada++;
    }
    entrada++; // Pula o "}"
    return obter_atomo(); // Retorna o próximo token válido
}

// Analisador Sintático

void erro_sintatico(TAtomo esperado) {
    printf("\n# ERRO SINTATICO na linha %d: Esperado '%s', mas encontrado '%s'\n", lookahead.linha, strAtomo[esperado], strAtomo[lookahead.atomo]);
    if (inicio_buffer) free(inicio_buffer);
    exit(1);
}

void erro_lexico() {
    printf("\n# ERRO Lexico na linha %d: %s\n", lookahead.linha, lookahead.lexema);
    if (inicio_buffer) free(inicio_buffer);
    exit(1);
}

void erro_geral(const char* mensagem) {
    printf("\n# ERRO SINTATICO na linha %d: %s\n", lookahead.linha, mensagem);
    if (inicio_buffer) free(inicio_buffer);
    exit(1);
}

void consome(TAtomo atomo) {
    if (lookahead.atomo == ERROLEXICO) {
        erro_lexico();
    }
    if (lookahead.atomo == atomo) {
        printf("%d %s", lookahead.linha, strAtomo[lookahead.atomo]);
        if(lookahead.atomo == IDENTIFICADOR || lookahead.atomo == NUMERO){
            printf(" | %s", lookahead.lexema);
        }
        printf("\n");
        lookahead = obter_atomo();
    } else {
        erro_sintatico(atomo);
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

// <block> ::= { <variable_declaration_part> | <procedure_declaration_part> } <statement_part>
void block() {
    while (lookahead.atomo == VAR || lookahead.atomo == PROCEDURE) {
        if (lookahead.atomo == VAR) {
            variable_declaration_part();
        } else if (lookahead.atomo == PROCEDURE) {
            procedure_declaration_part();
        }
    }
    if (lookahead.atomo != BEGIN) {
        if (lookahead.atomo == IDENTIFICADOR) {
            erro_geral("Palavra-chave 'var' ausente antes da declaracao.");
        } else {
            erro_sintatico(BEGIN); 
        }
    }
    statement_part();
}

// <variable_declaration_part> ::= var <variable_declaration> {; <variable_declaration>} ; | <>
void variable_declaration_part() {
    consome(VAR);
    if (lookahead.atomo == ERROLEXICO) {
        erro_lexico();
    }
    while (lookahead.atomo == IDENTIFICADOR) {
        variable_declaration();
        consome(PONTO_VIRGULA);
    }
}

// <procedure_declaration_part> ::= procedure <identifier> [ ( <parameter_list> ) ] ; <block> ;
void procedure_declaration_part() {
    consome(PROCEDURE);
    consome(IDENTIFICADOR);

    // Parte opcional de parâmetros: [ ( <parameter_list> ) ]
    if (lookahead.atomo == ABRE_PARENTESES) {
        consome(ABRE_PARENTESES);

        if (lookahead.atomo == VAR) {
            parameter_list();

            // Permitir mais parâmetros separados por '; var ...'
            while (lookahead.atomo == PONTO_VIRGULA) {
                consome(PONTO_VIRGULA);
                parameter_list();
            }
        }

        consome(FECHA_PARENTESES);
    }

    consome(PONTO_VIRGULA);
    block();
    consome(PONTO_VIRGULA);
}

// <parameter_list> ::= var <identifier> : <type> { ; var <identifier> : <type> }
void parameter_list() {
    consome(VAR);
    consome(IDENTIFICADOR);
    consome(DOIS_PONTOS);
    type();
}

// <variable_declaration> ::= <identifier> {, <identifier>} : <type>
void variable_declaration() {
    consome(IDENTIFICADOR);
    if (lookahead.atomo == ATRIBUICAO) {
        erro_geral("Atribuicao (:=) invalida dentro de um bloco 'var'.");
    }
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
        erro_geral("Tipo invalido (esperado 'integer' ou 'boolean').");
    }
}

// <statement_part> ::= begin <statement> {; <statement>} end
void statement_part() {
    consome(BEGIN);
    statement();
    while (lookahead.atomo != END) {
        if (lookahead.atomo == BEGIN) {
            erro_geral("'begin' inesperado, possivel 'end' ausente.");
        }
        consome(PONTO_VIRGULA);
        if (lookahead.atomo == END) {
            break;
        }
        statement();
    }
    consome(END);
}

// <statement> ::= <assignment_statement> | <if_statement> | <while_statement> | <write_statement> | <statement_part> | <>
void statement() {
    switch (lookahead.atomo) {
        case IDENTIFICADOR: assignment_statement(); break;
        case IF: if_statement(); break;
        case WHILE: while_statement(); break;
        case WRITE: write_statement(); break;
        case BEGIN: statement_part(); break; // Bloco aninhado
        default:
            if (lookahead.atomo != END) {
                erro_geral("Comando invalido.");
            }
    }
}

// <assignment_statement> ::= <identifier> := <expression>
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

// <write_statement> ::= write ( <expression> )
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

// <simple_expression> ::= [ + | - ] <term> { (+ | - | or) <term> }
void simple_expression() {
    term();
    while (lookahead.atomo == MAIS || lookahead.atomo == MENOS || lookahead.atomo == OU_LOGICO) {
        consome(lookahead.atomo);
        term();
    }
}

// <term> ::= <factor> { (* | div | and) <factor> }
void term() {
    factor();
    while (lookahead.atomo == MULT || lookahead.atomo == DIV || lookahead.atomo == E_LOGICO) {
        consome(lookahead.atomo);
        factor();
    }
}

// <factor> ::= <identifier> | <numero> | <boolean> | ( <expression> ) | not <factor>
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
            erro_geral("Fator invalido na expressao.");
    }
}

// Retorna o token do operador relacional se existir
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
