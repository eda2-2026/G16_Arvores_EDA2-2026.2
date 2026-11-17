#ifndef PRODUTO_HPP
#define PRODUTO_HPP

#include <string>

// Item do estoque. O campo `codigo` é a chave usada na árvore rubro-negra.
struct Produto {
    std::string codigo;
    std::string nome;
    std::string categoria;
    int quantidade;
    double preco;
};

#endif
