# Gerenciador de estoque com árvore rubro-negra

Gerenciador de estoque em C++17 com interface gráfica nativa para Windows.
Os produtos são indexados pelo código em uma árvore rubro-negra, mantendo as
operações de busca, inserção e remoção em O(log n).

## Interface gráfica

Com `g++` do MinGW, compile e execute:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -D ESTOQUE_GUI src\main.cpp -mwindows -lcomctl32 -lcomdlg32 -o estoque.exe
.\estoque.exe
```

Na janela, use **Novo produto** para cadastrar, selecione um produto para
atualizar a quantidade ou removê-lo, e use a busca para filtrar por código,
nome ou categoria. Um duplo clique no produto também abre a atualização de
quantidade. **Importar CSV** abre um seletor de arquivo e exibe o resultado da
importação. O painel mostra o número de produtos, as unidades em estoque e
quantos itens têm cinco unidades ou menos.

## Versão de console

O modo de console também continua disponível:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic src\main.cpp -o estoque.exe
.\estoque.exe
```

## Importar estoque por CSV

No modo de console, escolha **6. Importar CSV** e informe o caminho do arquivo.
Na interface gráfica, use **Importar CSV**. O arquivo deve usar UTF-8 e ter
este cabeçalho, nesta ordem:

```csv
codigo,nome,categoria,quantidade,preco
001,Teclado,"Perifericos, USB",12,149.90
002,Mouse,Perifericos,25,79.50
```

Campos de texto com vírgula devem ficar entre aspas; aspas dentro desses campos
são escapadas como `""`. Quantidades devem ser inteiros não negativos e preços
devem usar ponto como separador decimal. Linhas inválidas e códigos já
cadastrados são ignorados com aviso; os demais produtos são inseridos na árvore.

Os dados ficam na memória durante a execução. Para carregar um estoque
novamente, importe o CSV ao iniciar o programa.