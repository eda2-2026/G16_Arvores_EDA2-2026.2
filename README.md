# Gerenciador de Estoque com Árvore Rubro-Negra

Trabalho de **Estruturas de Dados 2 (EDA2) – Árvores**, grupo **G16**.

Aplicação em C++ para cadastrar e controlar produtos de um estoque. Os produtos
ficam numa **árvore rubro-negra** indexada pelo código, então busca, inserção
e remoção custam **O(log n)** mesmo quando os códigos chegam em ordem.
O programa tem interface gráfica (Win32) e interface de console, importa e
exporta CSV e salva o estoque automaticamente.

## Integrantes

| Nome                  | Matrícula |
| --------------------- | --------- |
| Pablo Cunha de Jesus  | 222014910 |
| Bryan                 | 231012156 |

## Vídeo de apresentação

[![Assista no YouTube](https://img.shields.io/badge/YouTube-Assistir%20apresenta%C3%A7%C3%A3o-red?logo=youtube)](https://youtu.be/9ZxQndBk9MY)

> Link direto: [LINK_DO_VIDEO_AQUI](https://youtu.be/9ZxQndBk9MY)

## Funcionalidades

- Cadastrar, consultar, editar (nome, categoria, quantidade e preço) e remover produtos.
- Atualizar só a quantidade em estoque.
- Busca por código, nome ou categoria (GUI).
- Painel com total de produtos, unidades em estoque, itens com estoque baixo
  (≤ 5 unidades) e altura atual da árvore.
- **Visualização da árvore**: desenho da estrutura com a cor de cada nó, altura,
  altura negra e verificação das propriedades rubro-negras.
- Importação e exportação de CSV.
- **Persistência automática** em `estoque.csv`.

## A árvore rubro-negra

Uma árvore rubro-negra é uma árvore binária de busca balanceada. Cada nó é
vermelho ou preto, e a árvore sempre respeita estas regras:

1. A raiz é preta.
2. As folhas nulas (aqui, um único **nó sentinela**) são pretas.
3. Um nó vermelho não tem filho vermelho.
4. Todo caminho de um nó até as folhas abaixo dele passa pelo mesmo número de
   nós pretos (a **altura negra**).

Por causa dessas regras, a altura nunca passa de `2·log₂(n+1)`. Depois de uma
inserção ou remoção, a árvore se reequilibra com **recolorações** e **rotações**
(esquerda e direita), seguindo o algoritmo de Cormen et al. (*Introduction to
Algorithms*).

| Operação                 | Complexidade |
| ------------------------ | ------------ |
| Busca por código         | O(log n)     |
| Inserção                 | O(log n)     |
| Remoção                  | O(log n)     |
| Atualização (por código) | O(log n)     |
| Listagem ordenada        | O(n)         |

**Por que rubro-negra?** Num estoque, os códigos costumam ser cadastrados em
sequência (001, 002, 003…). Uma árvore binária de busca comum viraria uma lista
nesse caso (altura n). A rubro-negra continua balanceada e faz menos rotações
na remoção do que a AVL. O teste automático insere 1023 códigos em ordem
crescente e confere que a altura fica dentro do limite.

## Estrutura do projeto

```
include/                     Cabeçalhos do núcleo (independente de interface)
  produto.hpp                struct Produto
  arvore_rubro_negra.hpp     classe ArvoreRubroNegra
  csv.hpp                    importação/exportação CSV
  persistencia.hpp           carregar/salvar estoque.csv
  utilitarios.hpp            conversões e validação de texto
src/
  arvore_rubro_negra.cpp     inserção, remoção, rotações, validação, impressão
  csv.cpp
  persistencia.cpp
  utilitarios.cpp
  console/main_console.cpp   interface de texto
  gui/                       interface gráfica Win32
    main_gui.cpp             janela principal (painel, tabela, botões)
    formulario.cpp/.hpp      formulário modal (novo, editar, quantidade)
    janela_arvore.cpp/.hpp   janela de visualização da árvore
    gui_comum.cpp/.hpp       cores, fonte e funções auxiliares
tests/teste_arvore.cpp       testes automáticos
dados/exemplo.csv            CSV de exemplo para demonstração
Makefile
```

## Como compilar e executar

É preciso ter o **g++ com suporte a C++17**. No Windows, use o MinGW-w64
(por exemplo, pelo [MSYS2](https://www.msys2.org/): `pacman -S mingw-w64-ucrt-x86_64-gcc`).

### Com Makefile

```powershell
mingw32-make            # compila GUI, console e testes
mingw32-make gui        # só a interface gráfica  -> estoque.exe
mingw32-make console    # só o console            -> estoque_console.exe
mingw32-make testar     # compila e roda os testes
```

### Sem Makefile

Interface gráfica:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src\arvore_rubro_negra.cpp src\csv.cpp src\persistencia.cpp src\utilitarios.cpp src\gui\main_gui.cpp src\gui\gui_comum.cpp src\gui\formulario.cpp src\gui\janela_arvore.cpp -mwindows -lcomctl32 -lcomdlg32 -o estoque.exe
.\estoque.exe
```

Console:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src\arvore_rubro_negra.cpp src\csv.cpp src\persistencia.cpp src\utilitarios.cpp src\console\main_console.cpp -o estoque_console.exe
.\estoque_console.exe
```

Testes:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src\arvore_rubro_negra.cpp src\csv.cpp src\persistencia.cpp src\utilitarios.cpp tests\teste_arvore.cpp -o teste_arvore.exe
.\teste_arvore.exe
```

## Como usar

### Interface gráfica

- **+ Novo produto**: abre o formulário de cadastro.
- **Editar**: altera nome, categoria, quantidade e preço do produto selecionado.
  O código é a chave da árvore e não pode ser alterado. Um duplo clique na
  linha faz o mesmo.
- **Atualizar quantidade**: muda só a quantidade em estoque.
- **Remover**: exclui o produto selecionado, com confirmação.
- **Importar CSV** e **Exportar CSV**: abrem o seletor de arquivo.
- **Ver árvore**: abre uma janela com a estrutura da árvore, que se atualiza a
  cada alteração.
- **Buscar**: filtra a tabela por código, nome ou categoria.

### Console

```
1. Cadastrar produto      6. Remover produto
2. Consultar produto      7. Importar CSV
3. Listar estoque         8. Exportar CSV
4. Atualizar quantidade   9. Mostrar arvore
5. Editar produto         0. Sair
```

Exemplo da opção **9** depois de inserir 010, 020, 030, 040, 050 e 025:

```
020 (P)
|-- E: 010 (P)
`-- D: 040 (V)
    |-- E: 030 (P)
    |   `-- E: 025 (V)
    `-- D: 050 (P)
```

`(P)` = preto, `(V)` = vermelho, `E:`/`D:` = filho esquerdo/direito.

## Persistência

Os dados ficam em `estoque.csv`, na pasta onde o programa é executado. O
arquivo é carregado ao abrir e regravado a cada alteração. A gravação usa um
arquivo temporário, para que o original não seja corrompido se algo falhar.
Se `estoque.csv` estiver corrompido, o programa faz uma cópia em
`estoque.csv.bak` antes de sobrescrevê-lo e mostra um aviso.

## Formato do CSV

UTF-8, com este cabeçalho exatamente nesta ordem:

```csv
codigo,nome,categoria,quantidade,preco
001,Teclado,"Perifericos, USB",12,149.90
002,Mouse,Perifericos,25,79.50
```

- Campos com vírgula ficam entre aspas, e aspas dentro do campo são escritas
  como `""`.
- A quantidade é um inteiro não negativo. O preço usa ponto como separador
  decimal.
- Na importação, linhas inválidas e códigos já cadastrados são ignorados com
  aviso, e os demais produtos entram na árvore.
- Há um arquivo pronto para teste em [`dados/exemplo.csv`](dados/exemplo.csv).

## Testes

`tests/teste_arvore.cpp` verifica:

- árvore vazia e operações básicas (inserir, buscar, atualizar, remover, duplicados);
- balanceamento: 1023 inserções em ordem crescente com altura ≤ 2·log₂(n+1);
- **10 000 operações aleatórias** de inserção e remoção, comparadas com `std::set`
  e checadas com `validar()` (ordem de busca, ponteiros de pai, raiz preta,
  nenhum vermelho com filho vermelho, mesma altura negra) depois de cada operação;
- parser CSV (aspas e escapes) e ida e volta exportar → importar.
