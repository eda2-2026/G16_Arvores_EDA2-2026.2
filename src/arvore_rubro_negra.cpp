#include "arvore_rubro_negra.hpp"

#include <algorithm>
#include <ostream>

ArvoreRubroNegra::ArvoreRubroNegra() : sentinela(nullptr), raiz(nullptr), totalNos(0) {
    sentinela = new No(Produto{"", "", "", 0, 0.0}, PRETO);
    sentinela->esquerda = sentinela;
    sentinela->direita = sentinela;
    sentinela->pai = sentinela;
    raiz = sentinela;
}

ArvoreRubroNegra::~ArvoreRubroNegra() {
    liberarNos(raiz);
    delete sentinela;
}

// ---------------------------------------------------------------------------
// Operações básicas
// ---------------------------------------------------------------------------

ArvoreRubroNegra::No* ArvoreRubroNegra::buscarNo(const std::string& codigo) const {
    No* atual = raiz;
    while (atual != sentinela && codigo != atual->produto.codigo) {
        atual = codigo < atual->produto.codigo ? atual->esquerda : atual->direita;
    }
    return atual;
}

bool ArvoreRubroNegra::inserir(const Produto& produto) {
    No* pai = sentinela;
    No* atual = raiz;
    while (atual != sentinela) {
        pai = atual;
        if (produto.codigo == atual->produto.codigo) {
            return false;
        }
        atual = produto.codigo < atual->produto.codigo ? atual->esquerda : atual->direita;
    }

    No* novo = new No(produto);
    novo->esquerda = sentinela;
    novo->direita = sentinela;
    novo->pai = pai;
    if (pai == sentinela) {
        raiz = novo;
    } else if (produto.codigo < pai->produto.codigo) {
        pai->esquerda = novo;
    } else {
        pai->direita = novo;
    }
    corrigirInsercao(novo);
    ++totalNos;
    return true;
}

const Produto* ArvoreRubroNegra::buscar(const std::string& codigo) const {
    No* no = buscarNo(codigo);
    return no == sentinela ? nullptr : &no->produto;
}

bool ArvoreRubroNegra::atualizarQuantidade(const std::string& codigo, int quantidade) {
    No* no = buscarNo(codigo);
    if (no == sentinela) {
        return false;
    }
    no->produto.quantidade = quantidade;
    return true;
}

bool ArvoreRubroNegra::atualizar(const Produto& produto) {
    No* no = buscarNo(produto.codigo);
    if (no == sentinela) {
        return false;
    }
    // A chave não muda, então a posição do nó na árvore continua válida.
    no->produto = produto;
    return true;
}

bool ArvoreRubroNegra::remover(const std::string& codigo) {
    No* alvo = buscarNo(codigo);
    if (alvo == sentinela) {
        return false;
    }

    No* removido = alvo;
    Cor corOriginal = removido->cor;
    No* filho;
    if (alvo->esquerda == sentinela) {
        filho = alvo->direita;
        substituir(alvo, alvo->direita);
    } else if (alvo->direita == sentinela) {
        filho = alvo->esquerda;
        substituir(alvo, alvo->esquerda);
    } else {
        removido = menor(alvo->direita);
        corOriginal = removido->cor;
        filho = removido->direita;
        if (removido->pai == alvo) {
            filho->pai = removido;
        } else {
            substituir(removido, removido->direita);
            removido->direita = alvo->direita;
            removido->direita->pai = removido;
        }
        substituir(alvo, removido);
        removido->esquerda = alvo->esquerda;
        removido->esquerda->pai = removido;
        removido->cor = alvo->cor;
    }
    delete alvo;
    if (corOriginal == PRETO) {
        corrigirRemocao(filho);
    }
    sentinela->pai = sentinela;
    --totalNos;
    return true;
}

std::vector<Produto> ArvoreRubroNegra::produtosEmOrdem() const {
    std::vector<Produto> produtos;
    produtos.reserve(totalNos);
    coletarEmOrdem(raiz, produtos);
    return produtos;
}

std::size_t ArvoreRubroNegra::tamanho() const {
    return totalNos;
}

bool ArvoreRubroNegra::vazia() const {
    return raiz == sentinela;
}

// ---------------------------------------------------------------------------
// Balanceamento
// ---------------------------------------------------------------------------

void ArvoreRubroNegra::rotacionarEsquerda(No* no) {
    No* filho = no->direita;
    no->direita = filho->esquerda;
    if (filho->esquerda != sentinela) {
        filho->esquerda->pai = no;
    }
    filho->pai = no->pai;
    if (no->pai == sentinela) {
        raiz = filho;
    } else if (no == no->pai->esquerda) {
        no->pai->esquerda = filho;
    } else {
        no->pai->direita = filho;
    }
    filho->esquerda = no;
    no->pai = filho;
}

void ArvoreRubroNegra::rotacionarDireita(No* no) {
    No* filho = no->esquerda;
    no->esquerda = filho->direita;
    if (filho->direita != sentinela) {
        filho->direita->pai = no;
    }
    filho->pai = no->pai;
    if (no->pai == sentinela) {
        raiz = filho;
    } else if (no == no->pai->direita) {
        no->pai->direita = filho;
    } else {
        no->pai->esquerda = filho;
    }
    filho->direita = no;
    no->pai = filho;
}

void ArvoreRubroNegra::corrigirInsercao(No* no) {
    while (no->pai->cor == VERMELHO) {
        if (no->pai == no->pai->pai->esquerda) {
            No* tio = no->pai->pai->direita;
            if (tio->cor == VERMELHO) {
                no->pai->cor = PRETO;
                tio->cor = PRETO;
                no->pai->pai->cor = VERMELHO;
                no = no->pai->pai;
            } else {
                if (no == no->pai->direita) {
                    no = no->pai;
                    rotacionarEsquerda(no);
                }
                no->pai->cor = PRETO;
                no->pai->pai->cor = VERMELHO;
                rotacionarDireita(no->pai->pai);
            }
        } else {
            No* tio = no->pai->pai->esquerda;
            if (tio->cor == VERMELHO) {
                no->pai->cor = PRETO;
                tio->cor = PRETO;
                no->pai->pai->cor = VERMELHO;
                no = no->pai->pai;
            } else {
                if (no == no->pai->esquerda) {
                    no = no->pai;
                    rotacionarDireita(no);
                }
                no->pai->cor = PRETO;
                no->pai->pai->cor = VERMELHO;
                rotacionarEsquerda(no->pai->pai);
            }
        }
    }
    raiz->cor = PRETO;
}

void ArvoreRubroNegra::substituir(No* antigo, No* novo) {
    if (antigo->pai == sentinela) {
        raiz = novo;
    } else if (antigo == antigo->pai->esquerda) {
        antigo->pai->esquerda = novo;
    } else {
        antigo->pai->direita = novo;
    }
    novo->pai = antigo->pai;
}

ArvoreRubroNegra::No* ArvoreRubroNegra::menor(No* no) const {
    while (no->esquerda != sentinela) {
        no = no->esquerda;
    }
    return no;
}

void ArvoreRubroNegra::corrigirRemocao(No* no) {
    while (no != raiz && no->cor == PRETO) {
        if (no == no->pai->esquerda) {
            No* irmao = no->pai->direita;
            if (irmao->cor == VERMELHO) {
                irmao->cor = PRETO;
                no->pai->cor = VERMELHO;
                rotacionarEsquerda(no->pai);
                irmao = no->pai->direita;
            }
            if (irmao->esquerda->cor == PRETO && irmao->direita->cor == PRETO) {
                irmao->cor = VERMELHO;
                no = no->pai;
            } else {
                if (irmao->direita->cor == PRETO) {
                    irmao->esquerda->cor = PRETO;
                    irmao->cor = VERMELHO;
                    rotacionarDireita(irmao);
                    irmao = no->pai->direita;
                }
                irmao->cor = no->pai->cor;
                no->pai->cor = PRETO;
                irmao->direita->cor = PRETO;
                rotacionarEsquerda(no->pai);
                no = raiz;
            }
        } else {
            No* irmao = no->pai->esquerda;
            if (irmao->cor == VERMELHO) {
                irmao->cor = PRETO;
                no->pai->cor = VERMELHO;
                rotacionarDireita(no->pai);
                irmao = no->pai->esquerda;
            }
            if (irmao->direita->cor == PRETO && irmao->esquerda->cor == PRETO) {
                irmao->cor = VERMELHO;
                no = no->pai;
            } else {
                if (irmao->esquerda->cor == PRETO) {
                    irmao->direita->cor = PRETO;
                    irmao->cor = VERMELHO;
                    rotacionarEsquerda(irmao);
                    irmao = no->pai->esquerda;
                }
                irmao->cor = no->pai->cor;
                no->pai->cor = PRETO;
                irmao->esquerda->cor = PRETO;
                rotacionarDireita(no->pai);
                no = raiz;
            }
        }
    }
    no->cor = PRETO;
}

// ---------------------------------------------------------------------------
// Percursos e liberação
// ---------------------------------------------------------------------------

void ArvoreRubroNegra::coletarEmOrdem(No* no, std::vector<Produto>& produtos) const {
    if (no == sentinela) {
        return;
    }
    coletarEmOrdem(no->esquerda, produtos);
    produtos.push_back(no->produto);
    coletarEmOrdem(no->direita, produtos);
}

void ArvoreRubroNegra::liberarNos(No* no) {
    if (no == sentinela) {
        return;
    }
    liberarNos(no->esquerda);
    liberarNos(no->direita);
    delete no;
}

// ---------------------------------------------------------------------------
// Métricas, validação e visualização
// ---------------------------------------------------------------------------

int ArvoreRubroNegra::alturaNo(const No* no) const {
    if (no == sentinela) {
        return 0;
    }
    return 1 + (std::max)(alturaNo(no->esquerda), alturaNo(no->direita));
}

int ArvoreRubroNegra::altura() const {
    return alturaNo(raiz);
}

int ArvoreRubroNegra::alturaNegra() const {
    int pretos = 0;
    for (const No* no = raiz; no != sentinela; no = no->esquerda) {
        if (no->cor == PRETO) {
            ++pretos;
        }
    }
    return pretos;
}

// Retorna a altura negra da subárvore (contando o sentinela) ou -1 em caso de erro.
int ArvoreRubroNegra::validarNo(const No* no, const std::string* minimo, const std::string* maximo,
                                std::size_t& contados, std::string& erro) const {
    if (no == sentinela) {
        return 1;
    }
    ++contados;
    const std::string& codigo = no->produto.codigo;
    if ((minimo != nullptr && !(*minimo < codigo)) || (maximo != nullptr && !(codigo < *maximo))) {
        erro = "ordem de busca violada no codigo '" + codigo + "'";
        return -1;
    }
    if (no->cor == VERMELHO && (no->esquerda->cor == VERMELHO || no->direita->cor == VERMELHO)) {
        erro = "no vermelho com filho vermelho no codigo '" + codigo + "'";
        return -1;
    }
    if ((no->esquerda != sentinela && no->esquerda->pai != no) ||
        (no->direita != sentinela && no->direita->pai != no)) {
        erro = "ponteiro de pai inconsistente abaixo do codigo '" + codigo + "'";
        return -1;
    }
    const int esquerda = validarNo(no->esquerda, minimo, &codigo, contados, erro);
    if (esquerda < 0) {
        return -1;
    }
    const int direita = validarNo(no->direita, &codigo, maximo, contados, erro);
    if (direita < 0) {
        return -1;
    }
    if (esquerda != direita) {
        erro = "alturas negras diferentes nas subarvores do codigo '" + codigo + "'";
        return -1;
    }
    return esquerda + (no->cor == PRETO ? 1 : 0);
}

bool ArvoreRubroNegra::validar(std::string* erro) const {
    std::string mensagem;
    std::size_t contados = 0;
    if (sentinela->cor != PRETO) {
        mensagem = "sentinela nao e preto";
    } else if (raiz->cor != PRETO) {
        mensagem = "raiz nao e preta";
    } else if (raiz != sentinela && raiz->pai != sentinela) {
        mensagem = "pai da raiz deveria ser o sentinela";
    } else if (validarNo(raiz, nullptr, nullptr, contados, mensagem) >= 0 && contados != totalNos) {
        mensagem = "contador de nos inconsistente";
    }
    if (erro != nullptr) {
        *erro = mensagem;
    }
    return mensagem.empty();
}

void ArvoreRubroNegra::imprimirNo(const No* no, std::ostream& saida, const std::string& prefixo,
                                  bool ultimo, const char* lado) const {
    saida << prefixo << (ultimo ? "`-- " : "|-- ") << lado << no->produto.codigo
          << (no->cor == PRETO ? " (P)" : " (V)") << '\n';
    const std::string prefixoFilhos = prefixo + (ultimo ? "    " : "|   ");
    const bool temEsquerda = no->esquerda != sentinela;
    const bool temDireita = no->direita != sentinela;
    if (temEsquerda) {
        imprimirNo(no->esquerda, saida, prefixoFilhos, !temDireita, "E: ");
    }
    if (temDireita) {
        imprimirNo(no->direita, saida, prefixoFilhos, true, "D: ");
    }
}

void ArvoreRubroNegra::imprimirEstrutura(std::ostream& saida) const {
    if (raiz == sentinela) {
        saida << "(arvore vazia)\n";
        return;
    }
    saida << raiz->produto.codigo << (raiz->cor == PRETO ? " (P)" : " (V)") << '\n';
    const bool temEsquerda = raiz->esquerda != sentinela;
    const bool temDireita = raiz->direita != sentinela;
    if (temEsquerda) {
        imprimirNo(raiz->esquerda, saida, "", !temDireita, "E: ");
    }
    if (temDireita) {
        imprimirNo(raiz->direita, saida, "", true, "D: ");
    }
}
