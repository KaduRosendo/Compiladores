# Capítulo 1 - fundamentos do Processo de Compilação

## Exercícios Teóricos 

### 1. Qual é a diferença entre um compilador e um interpretador ?

R: 

O compilador traduz o programa inteiro e retorna um arquivo executavel, já o interpretador executa o programa enquanto traduz, linha por linha  

### 2. Quais são as vantagens de (a) um compilador em relação a um interpretador e (b) um interpretador em relação a um compilador?

R: 

(a) Velocidade de execução, como já traduz tudo antes, o programa roda mais rápido. Otimização, compiladores fazem ajustes que deixam o programa mais eficiente.

(b) Facilita depuração, porque você pode executar linha por linha e ver onde esta o erro. Rapidez no teste, não precisa compilar tudo, pois roda o código apos escrever.

### 3. Que vantagens existem em um sistema de processamento de linguagem no qual o compilador produz linguagem simbólica em vez de linguagem de máquina?
R:

É muito mais fácil para o compilador gerar uma linguagem sombólica como programa fonte e apartir dai usar um tradutor para gerar a linguagem de máquina

### 4. Um compilador que traduz uma linguagem de alto nível para outra linguagem de alto nível é chamado de tradutor de fonte para fonte. Que vantagens existem em usar C como linguagem objeto para um compilador?
R:

Quando traduzimos alto nível para alto nível está mantendo o mesmo programa fonte um pouco mais fácil com C 

### 5. Explique as fases que compõe a parte de análise e as fases que compõe a fase de síntese em um processo de compilação.
R:

Verifica a corretude do código de uma forma geral e depois as etapas que geram aquele programa objeto 
### 6. Procure exemplos de linguagens interpretadas e linguagens compiladas.
R:

Compiladas: C / Rust / Swift 
Interpretadas: Python / JavaScript / Ruby

### 7. O que são compiladores just-in-time? De um exemplo de linguagem de programação com essa característica.
R:

Es compilam o código em tempo de execução. O código primeiro convertido em bytecode. Durante a execução, identifica trechos são então compilados para código de máquina nativo. Se o comportamento do programa mudar, o JIT pode recompilar partes do código com novas otimizações. Java e C# são exemplos de linguagens 
### 8. O que é um pré-processador? Explique como ele trabalha em conjunto com o compilador.
R:

O pré-processador é um programa que manipula o texto do código antes da compilação. Ele não executa análise semântica profunda 
### 9. Explique cada uma das fases de um compilador e como se interagem entre si.
R:

Análise Léxica 
Análise Sintática 
Análise Semântica 
Gerador de um código Intermediário 
Otimização de Código 
Geração de Código

### 10. Qual tipo de software tradutor deve ser utilizado para programas em geral, quando a velocidade de execução é uma exigência de alta prioridade?
### (a)Compiladores
### (b)Interpretadores
### (c)Tradutores híbridos
### (d)Macro processadores
### (e)Interpretadores de macro instruções
R:

A-	Compiladores 
### 11. Um vendedor de artigos de pesca obteve com um amigo o código executável (já compilado) de um programa que gerencia vendas e faz o controle de estoque, com o intuito de usálo em sua loja. Segundo o seu amigo, o referido programa foi compilado em seu sistema computacional pessoal (sistema A) e funciona corretamente. O vendedor constatou que o programa executável também funciona corretamente no sistema computacional de sua loja (sistema B). Considerando a situação relatada, analise as afirmações a seguir.
### I. Os computadores poderiam ter quantidades diferentes de núcleos (cores).
### II. As chamadas ao sistema (system call) do sistema operacional no sistema A devem ser compatíveis com as do sistema B.
### III. O conjunto de instruções do sistema A poderia ser diferente do conjunto de instruções do sistema B.
### IV. Se os registradores do sistema A forem de 64 bits, os registradores do sistema B poderiam ser de 32 bits.
### É correto o que se afirma em:
### (a) III, apenas.
### (b)I e II, apenas.
### (c) III e IV, apenas.
### (d)I, II e IV, apenas.
### (e) I, II, III e IV.
R:

B-	I e II, apenas
