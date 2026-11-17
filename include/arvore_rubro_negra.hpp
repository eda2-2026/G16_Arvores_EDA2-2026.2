#ifndef ARVORE_RUBRO_NEGRA_HPP
#define ARVORE_RUBRO_NEGRA_HPP

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

#include "produto.hpp"

// Árvore rubro-negra indexada pelo código do produto.
// Usa um nó sentinela preto no lugar das folhas nulas (como em Cormen et al.),
// garantindo altura O(log n) e busca, inserção e remoção em O(log n).
class ArvoreRubroNegra {
public:
    ArvoreRubroNegra();
    ~ArvoreRubroNegra();

    ArvoreRubroNegra(const ArvoreRubroNegra&) = delete;
    ArvoreRubroNegra& operator=(const ArvoreRubroNegra&) = delete;

    // Insere o produto; retorna false se o código já existir.
    bool inserir(const Produto& produto);

    // Retorna o produto com o código informado ou nullptr se não existir.
    const Produto* buscar(const std::string& codigo) const;

    bool atualizarQuantidade(const std::string& codigo, int quantidade);

    // Localiza pelo código e substitui nome, categoria, quantidade e preço.
    bool atualizar(const Produto& produto);

    // Remove o produto; retorna false se o código não existir.
    bool remover(const std::string& codigo);

    // Produtos ordenados pelo código (percurso em ordem).
    std::vector<Produto> produtosEmOrdem() const;

    std::size_t tamanho() const;
    bool vazia() const;

    // Número de nós no maior caminho da raiz até uma folha (0 se vazia).
    int altura() const;

    // Número de nós pretos da raiz até uma folha, sem contar o sentinela.
    int alturaNegra() const;

    // Verifica a ordem de busca, os ponteiros de pai e as propriedades rubro-negras.
    // Em caso de violação, retorna false e descreve o problema em `erro`.
    bool validar(std::string* erro = nullptr) const;

    // Desenha a estrutura da árvore em texto, com a cor de cada nó.
    void imprimirEstrutura(std::ostream& saida) const;

private:
    enum Cor { VERMELHO, PRETO };

    struct No {
        Produto produto;
        Cor cor;
        No* esquerda;
        No* direita;
        No* pai;

        explicit No(const Produto& produto, Cor cor = VERMELHO)
            : produto(produto), cor(cor), esquerda(nullptr), direita(nullptr), pai(nullptr) {}
    };

    No* sentinela;
    No* raiz;
    std::size_t totalNos;

    No* buscarNo(const std::string& codigo) const;
    void rotacionarEsquerda(No* no);
    void rotacionarDireita(No* no);
    void corrigirInsercao(No* no);
    void substituir(No* antigo, No* novo);
    No* menor(No* no) const;
    void corrigirRemocao(No* no);
    void coletarEmOrdem(No* no, std::vector<Produto>& produtos) const;
    void liberarNos(No* no);
    int alturaNo(const No* no) const;
    int validarNo(const No* no, const std::string* minimo, const std::string* maximo,
                  std::size_t& contados, std::string& erro) const;
    void imprimirNo(const No* no, std::ostream& saida, const std::string& prefixo,
                    bool ultimo, const char* lado) const;
};

#endif
