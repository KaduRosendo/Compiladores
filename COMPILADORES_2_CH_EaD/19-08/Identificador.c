#include <stdio.h>
#include <ctype.h>

#define MAX_LINHAS 5000

int main () {
    char arquivo_entrada[] = "entrada.txt";
    char arquivo_saida[] = "saida.txt";

    FILE *entrada, *saida;

    entrada = fopen(arquivo_entrada, "r");
    if (entrada == NULL) {
        printf("Erro ao abrir o arquivo: '%s'", arquivo_entrada);
        return 1;
    }

    // Variaveis 
    int linhas = 1; 
    int vogais = 0, consoantes = 0, palavras = 0, espacos = 0;
    int maiusculas[26] = {0}, minusculas[26] = {0}, digitos[10] = {0};

    int palavras_linha[MAX_LINHAS] = {0};
    int linha_atual = 0;

    char c; 
    int na_palavra = 0;

    // Lê o arquivo 
    while ((c = fgetc(entrada)) != EOF) {
        // d) Quantidade de ocorrências de cada letra maiúscula do alfabeto
        if (isupper(c)) {
            maiusculas[c - 'A']++;
        }
        // e) Quantidade de ocorrências de cada letra minúscula do alfabeto
        else if (islower(c)) {
            minusculas[c - 'a']++;
        }
        // f) Quantidade de ocorrências de cada digito de 0 a 9
        else if (isdigit(c)) {
            digitos[c - '0']++;
        }
        // g) Quantidade de espaços em branco
        else if (c == ' ') {
            espacos++;
        }
        // a) Número de linhas do arquivo
        else if (c == '\n') {
            linhas++;
            linha_atual++;
            if (linha_atual >= MAX_LINHAS) {
                printf("O numero dde linhas excede o limite");
                linha_atual = MAX_LINHAS -1;
            }
        }

        char c_lower = tolower(c);
        // b) Quantidade de vogais
        if (c_lower == 'a' || c_lower == 'e' || c_lower == 'i' || c_lower == 'o' || c_lower == 'u') {
            vogais++;
        // c) Quantidade de consoantes
        }else if (isalpha(c_lower)) {
            consoantes++;
        }

        // i) Quantidade de palavras em cada linha do arquivo, considerando que as palavras são separadas por um ou mais espaços, vírgula, ponto ou ponto e vírgula
        // j) Quantidade de palavras no arquivo, considerando que as palavras são separadas por um ou mais espaços, vírgula, ponto ou ponto e vírgula
        if (isalpha(c)) {
            if (na_palavra == 0) {
                na_palavra = 1;
                palavras++;
                palavras_linha[linha_atual]++;
            }
        } else if (isdigit(c)){

        } else {
            na_palavra = 0;
        }

    }

    fseek(entrada, 0, SEEK_END);
    if (ftell(entrada) == 0) {
        linhas = 0;
        palavras = 0;
    }

    rewind(entrada);

    // h) Arquivo de saída com o mesmo conteúdo, mas com todas as letras minúsculas convertidas em maiúsculas. Dica: toupper()
    saida = fopen(arquivo_saida, "w");
    if (saida == NULL) {
        printf("Erro ao criar o arquivo de saida\n");
        fclose(saida);
        return 1;
    }

    while ((c = fgetc(entrada)) != EOF) {
        fputc(toupper(c), saida);
    }

    fclose(entrada);
    fclose(saida);
        
    
    //Resultados 
    printf("(a) Numero de linhas: %d\n", linhas);
    printf("(b) Quantidade de vogais: %d\n", vogais);
    printf("(c) Quantidade de consoantes: %d\n", consoantes);
    printf("(d) Ocorrencia de letras maiusculas:\n");
    for (int i = 0; i < 26; i ++){
        if (maiusculas[i] > 0) {
            printf("    %c: %d\n", 'a' + i, maiusculas[i]);
        }
    }
    printf("(e) Ocorrencia de letras minusculas:\n");
    for (int i = 0; i < 26; i++) {
        if (minusculas[i] > 0) {
            printf("    %c: %d\n", 'a' + i, minusculas[i]);
        }
    }
        
    printf("(f) Ocorrencia de digitos:\n");
    for (int i = 0; i < 10; i++) {
        if (digitos[i] > 0) {
            printf("    %d: %d\n", i, digitos[i]);
        }
    }

    printf("(g) Espacos em branco: %d\n", espacos);
    printf("(h) arquivo de saida\n");
    printf("(i) Total de palavras\n");
    for (int i = 0; i < linhas && MAX_LINHAS; i++){
        printf("    Linha %d, %d\n", i + 1, palavras_linha[i]);
    }
        
    printf("(j) Total de palavras: %d\n", palavras);

    return 0;
}