program TesteCompleto;

var
  valorA, valorB, resultado : integer;
  condicao, flag : boolean;

/*
  Este eh um programa de teste para o compilador MiniPascal.
  Ele verifica diversas funcionalidades da linguagem.
*/

begin

  valorA := 10;
  valorB := 20;
  condicao := true;

  if (valorA < valorB) and condicao then
  begin
    resultado := valorB;  /* O resultado deve ser 20 */
    flag := true;
  end
  else
  begin
    resultado := valorA;
    flag := false;
  end;

  write(resultado);
  write(flag);

  /* Teste do laco de repeticao while */
  resultado := 5;
  while resultado > 0 do
  begin
    write(resultado);
    resultado := resultado - 1;
  end;

  /* Teste de operadores div e mult */
  valorA := 100;
  valorB := 10;
  resultado := (valorA div valorB) * 2; /* (100 div 10) * 2 = 20 */
  write(resultado);

end.
