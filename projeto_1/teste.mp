# Teste completo com código válido
vetorList = [2, 3, 5, 7, 11]
i = 0

while i < len(vetorList):
    print(vetorList[i])
    i = i + 1

number1 = input("digite um numero: ")
number2 = input("digite outro numero: ")

if number1 > number2:
    print("o maior numero eh ---> ", number1)
else:
    print(number1, " <= ", number2)

for j in range(50):
    if j % 2 == 0:
        print(j, " eh par")
    else:
        print(j, " eh impar")
