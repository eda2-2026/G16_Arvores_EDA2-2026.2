// Testes automáticos da árvore rubro-negra e do CSV (sem framework externo).
// Retorna 0 se tudo passar e 1 se alguma verificação falhar.

#include <cmath>
#include <cstdio>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "arvore_rubro_negra.hpp"
#include "csv.hpp"

namespace {

int falhas = 0;
int verificacoes = 0;

#define VERIFICAR(condicao)                                                          \
    do {                                                                             \
        ++verificacoes;                                                              \
        if (!(condicao)) {                                                           \
            ++falhas;                                                                \
            std::cerr << "FALHOU (" << __LINE__ << "): " #condicao << '\n';          \
        }                                                                            \
    } while (0)

Produto produto(const std::string& codigo, int quantidade = 1) {
    return Produto{codigo, "Produto " + codigo, "Teste", quantidade, 9.99};
}

std::string codigoAleatorio(std::mt19937& gerador) {
    std::uniform_int_distribution<int> distribuicao(0, 1999);
    char texto[16];
    std::snprintf(texto, sizeof(texto), "P%04d", distribuicao(gerador));
    return texto;
}

void testarArvoreVazia() {
    ArvoreRubroNegra arvore;
    VERIFICAR(arvore.vazia());
    VERIFICAR(arvore.tamanho() == 0);
    VERIFICAR(arvore.altura() == 0);
    VERIFICAR(arvore.alturaNegra() == 0);
    VERIFICAR(arvore.validar());
    VERIFICAR(arvore.buscar("X") == nullptr);
    VERIFICAR(!arvore.remover("X"));
    VERIFICAR(!arvore.atualizarQuantidade("X", 3));
}

void testarOperacoesBasicas() {
    ArvoreRubroNegra arvore;
    VERIFICAR(arvore.inserir(produto("B")));
    VERIFICAR(arvore.inserir(produto("A")));
    VERIFICAR(arvore.inserir(produto("C")));
    VERIFICAR(!arvore.inserir(produto("B")));  // código duplicado
    VERIFICAR(arvore.tamanho() == 3);
    VERIFICAR(arvore.validar());

    VERIFICAR(arvore.atualizarQuantidade("A", 42));
    VERIFICAR(arvore.buscar("A") != nullptr && arvore.buscar("A")->quantidade == 42);

    Produto editado{"C", "Novo nome", "Nova categoria", 7, 1.5};
    VERIFICAR(arvore.atualizar(editado));
    const Produto* c = arvore.buscar("C");
    VERIFICAR(c != nullptr && c->nome == "Novo nome" && c->categoria == "Nova categoria" &&
              c->quantidade == 7 && c->preco == 1.5);
    VERIFICAR(!arvore.atualizar(produto("Z")));

    const std::vector<Produto> ordem = arvore.produtosEmOrdem();
    VERIFICAR(ordem.size() == 3 && ordem[0].codigo == "A" && ordem[1].codigo == "B" && ordem[2].codigo == "C");

    VERIFICAR(arvore.remover("B"));
    VERIFICAR(arvore.buscar("B") == nullptr);
    VERIFICAR(arvore.tamanho() == 2);
    VERIFICAR(arvore.validar());
}

// Insere chaves crescentes: numa BST comum a altura seria n; na rubro-negra
// deve ficar abaixo de 2*log2(n+1).
void testarBalanceamento() {
    ArvoreRubroNegra arvore;
    const int total = 1023;
    char texto[16];
    for (int i = 0; i < total; ++i) {
        std::snprintf(texto, sizeof(texto), "%05d", i);
        arvore.inserir(produto(texto));
    }
    std::string erro;
    VERIFICAR(arvore.validar(&erro));
    VERIFICAR(arvore.altura() <= 2 * static_cast<int>(std::log2(total + 1)));
    std::cout << "  " << total << " insercoes em ordem crescente -> altura " << arvore.altura()
              << ", altura negra " << arvore.alturaNegra() << '\n';
}

// Compara a árvore com std::set após cada operação aleatória.
void testarOperacoesAleatorias() {
    std::mt19937 gerador(20262);
    std::uniform_int_distribution<int> operacao(0, 2);
    ArvoreRubroNegra arvore;
    std::set<std::string> referencia;
    const int total = 10000;
    bool consistente = true;
    std::string erro;
    for (int i = 0; i < total && consistente; ++i) {
        const std::string codigo = codigoAleatorio(gerador);
        if (operacao(gerador) < 2) {
            const bool inseriu = arvore.inserir(produto(codigo));
            consistente = inseriu == referencia.insert(codigo).second;
        } else {
            const bool removeu = arvore.remover(codigo);
            consistente = removeu == (referencia.erase(codigo) == 1);
        }
        consistente = consistente && arvore.validar(&erro) && arvore.tamanho() == referencia.size() &&
                      (arvore.buscar(codigo) != nullptr) == (referencia.count(codigo) == 1);
        if (!consistente) {
            std::cerr << "  divergencia na operacao " << i << " (codigo " << codigo << "): " << erro << '\n';
        }
    }
    VERIFICAR(consistente);

    const std::vector<Produto> ordem = arvore.produtosEmOrdem();
    VERIFICAR(ordem.size() == referencia.size());
    std::size_t indice = 0;
    bool mesmaOrdem = ordem.size() == referencia.size();
    for (const std::string& codigo : referencia) {
        if (!mesmaOrdem) {
            break;
        }
        mesmaOrdem = ordem[indice++].codigo == codigo;
    }
    VERIFICAR(mesmaOrdem);

    // Esvazia a árvore removendo tudo.
    for (const std::string& codigo : referencia) {
        arvore.remover(codigo);
    }
    VERIFICAR(arvore.vazia() && arvore.validar());
    std::cout << "  " << total << " operacoes aleatorias conferidas contra std::set\n";
}

void testarSeparacaoCsv() {
    std::vector<std::string> campos;
    VERIFICAR(separarCsv("001,Teclado,\"Perifericos, USB\",12,149.90", campos));
    VERIFICAR(campos.size() == 5 && campos[2] == "Perifericos, USB");
    VERIFICAR(separarCsv("002,\"Monitor 24\"\"\",Video,3,899", campos));
    VERIFICAR(campos.size() == 5 && campos[1] == "Monitor 24\"");
    VERIFICAR(!separarCsv("003,\"sem fechar,Video,3,899", campos));
    VERIFICAR(escaparCampoCsv("simples") == "simples");
    VERIFICAR(escaparCampoCsv("a,b") == "\"a,b\"");
    VERIFICAR(escaparCampoCsv("diz \"oi\"") == "\"diz \"\"oi\"\"\"");
}

void testarIdaEVoltaCsv() {
    const std::string caminho = "teste_ida_e_volta.csv";
    ArvoreRubroNegra original;
    original.inserir(Produto{"001", "Teclado", "Perifericos, USB", 12, 149.90});
    original.inserir(Produto{"002", "Monitor 24\"", "Video", 3, 899.0});
    original.inserir(Produto{"003", "Cabo \"HDMI\", 2m", "Cabos", 0, 25.5});

    std::ostringstream erros;
    VERIFICAR(exportarCsv(caminho, original, erros));

    ArvoreRubroNegra copia;
    std::ostringstream saida;
    ResultadoImportacao resultado;
    VERIFICAR(importarCsv(caminho, copia, saida, erros, &resultado));
    VERIFICAR(resultado.inseridos == 3 && resultado.ignorados == 0);

    const std::vector<Produto> a = original.produtosEmOrdem();
    const std::vector<Produto> b = copia.produtosEmOrdem();
    bool iguais = a.size() == b.size();
    for (std::size_t i = 0; iguais && i < a.size(); ++i) {
        iguais = a[i].codigo == b[i].codigo && a[i].nome == b[i].nome && a[i].categoria == b[i].categoria &&
                 a[i].quantidade == b[i].quantidade && std::fabs(a[i].preco - b[i].preco) < 0.005;
    }
    VERIFICAR(iguais);
    if (!erros.str().empty()) {
        std::cerr << erros.str();
    }
    std::remove(caminho.c_str());
}

void exibirExemplo() {
    ArvoreRubroNegra arvore;
    for (const char* codigo : {"010", "020", "030", "040", "050", "025"}) {
        arvore.inserir(produto(codigo));
    }
    std::cout << "  Exemplo de estrutura (insercoes 010, 020, 030, 040, 050, 025):\n";
    std::ostringstream estrutura;
    arvore.imprimirEstrutura(estrutura);
    std::istringstream linhas(estrutura.str());
    std::string linha;
    while (std::getline(linhas, linha)) {
        std::cout << "    " << linha << '\n';
    }
}

}  // namespace

int main() {
    std::cout << "Testes da arvore rubro-negra\n";
    testarArvoreVazia();
    testarOperacoesBasicas();
    testarBalanceamento();
    testarOperacoesAleatorias();
    std::cout << "Testes do CSV\n";
    testarSeparacaoCsv();
    testarIdaEVoltaCsv();
    exibirExemplo();

    std::cout << '\n' << (verificacoes - falhas) << '/' << verificacoes << " verificacoes passaram.\n";
    return falhas == 0 ? 0 : 1;
}
