Integrantes:

Carlos Eduardo Rosendo Basseto - 10409941

João Rocha Murgel - 10410293

O compilador criado realiza tanto a análise léxica quanto sintática de um arquivo de texto contendo um código na linguagem MiniPascal.

Parte Léxica:

As funções de verificação léxica foram construídas com base em autômatos para cada tipo de token. Caso a entrada não corresponda a um padrão válido da linguagem, será mostrado um erro léxico para o usuário. Além das funções para reconhecer números, identificadores e comentários, foi implementada uma lógica para agrupar caracteres que formam um único átomo, como no caso do operador de atribuição ':=', que é formado por dois caracteres distintos.

Na função reconhece_id_ou_palavra_reservada e reconhece_numero, o conteúdo do lexema é armazenado no campo correspondente da struct TInfoAtomo.
As principais funções de verificação léxica criadas no projeto são:

TInfoAtomo obter_atomo()

TInfoAtomo reconhece_id_ou_palavra_reservada()

TInfoAtomo reconhece_numero()

TInfoAtomo reconhece_comentario()

Parte Sintática:

Para a parte sintática, foram criadas funções de acordo com as regras de produção da gramática MiniPascal, fornecida no enunciado do projeto. A abordagem utilizada foi a de Análise Descendente Recursiva.

A função inicial é a program(), que é chamada na main e efetivamente constrói a árvore de derivação ao realizar chamadas recursivas das outras funções. Caso a sequência de átomos lida pelo analisador léxico não esteja em uma ordem válida, um erro sintático é retornado, informando o token esperado e o encontrado.

As funções de verificação sintática criadas no projeto são:

void program()

void block()

void variable_declaration_part()

void variable_declaration()

void type()

void statement_part()

void statement()

void assignment_statement()

void if_statement()

void while_statement()

void write_statement()

void expression()

void simple_expression()

void term()

void factor()

TAtomo relational_operator()
