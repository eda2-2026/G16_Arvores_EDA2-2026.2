// Interface de texto do gerenciador de estoque.

#include <iomanip>
#include <iostream>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "arvore_rubro_negra.hpp"
#include "csv.hpp"
#include "persistencia.hpp"
#include "utilitarios.hpp"

namespace {

std::string lerTexto(const std::string& mensagem) {
    std::cout << mensagem;
    std::string texto;
    std::getline(std::cin, texto);
    return aparar(texto);
}

bool lerQuantidade(const std::string& mensagem, int& quantidade) {
    const std::string texto = lerTexto(mensagem);
    return converterInteiro(texto, quantidade) && quantidade >= 0;
}

void imprimirCabecalho() {
    std::cout << std::left << std::setw(16) << "Codigo"
              << std::setw(28) << "Nome"
              << std::setw(20) << "Categoria"
              << std::right << std::setw(10) << "Quantidade"
              << std::setw(12) << "Preco" << '\n'
              << std::string(86, '-') << '\n';
}

void imprimirProduto(const Produto& produto) {
    std::cout << std::left << std::setw(16) << produto.codigo
              << std::setw(28) << produto.nome
              << std::setw(20) << produto.categoria
              << std::right << std::setw(10) << produto.quantidade
              << std::setw(12) << std::fixed << std::setprecision(2) << produto.preco
              << '\n';
}

bool lerProduto(Produto& produto) {
    produto.codigo = lerTexto("Codigo: ");
    produto.nome = lerTexto("Nome: ");
    produto.categoria = lerTexto("Categoria: ");
    if (produto.codigo.empty() || produto.nome.empty() || produto.categoria.empty()) {
        std::cout << "Codigo, nome e categoria sao obrigatorios.\n";
        return false;
    }
    if (!lerQuantidade("Quantidade: ", produto.quantidade)) {
        std::cout << "Quantidade invalida. Informe um inteiro nao negativo.\n";
        return false;
    }
    if (!converterPreco(lerTexto("Preco: "), produto.preco)) {
        std::cout << "Preco invalido. Informe um numero nao negativo usando ponto decimal.\n";
        return false;
    }
    return true;
}

void salvar(const ArvoreRubroNegra& estoque) {
    if (!salvarEstoque(estoque, std::cerr)) {
        std::cout << "Atencao: as alteracoes nao foram salvas em " << ARQUIVO_ESTOQUE << ".\n";
    }
}

void exibirMenu() {
    std::cout << "\n=== Gerenciador de Estoque (arvore rubro-negra) ===\n"
              << "1. Cadastrar produto\n"
              << "2. Consultar produto\n"
              << "3. Listar estoque\n"
              << "4. Atualizar quantidade\n"
              << "5. Editar produto\n"
              << "6. Remover produto\n"
              << "7. Importar CSV\n"
              << "8. Exportar CSV\n"
              << "9. Mostrar arvore\n"
              << "0. Sair\n";
}

void cadastrar(ArvoreRubroNegra& estoque) {
    Produto produto;
    if (!lerProduto(produto)) {
        return;
    }
    if (estoque.inserir(produto)) {
        std::cout << "Produto cadastrado.\n";
        salvar(estoque);
    } else {
        std::cout << "Ja existe um produto com esse codigo.\n";
    }
}

void consultar(const ArvoreRubroNegra& estoque) {
    const Produto* produto = estoque.buscar(lerTexto("Codigo: "));
    if (produto == nullptr) {
        std::cout << "Produto nao encontrado.\n";
        return;
    }
    imprimirCabecalho();
    imprimirProduto(*produto);
}

void listar(const ArvoreRubroNegra& estoque) {
    if (estoque.vazia()) {
        std::cout << "Estoque vazio.\n";
        return;
    }
    imprimirCabecalho();
    for (const Produto& produto : estoque.produtosEmOrdem()) {
        imprimirProduto(produto);
    }
    std::cout << estoque.tamanho() << " produto(s).\n";
}

void atualizarQuantidade(ArvoreRubroNegra& estoque) {
    const std::string codigo = lerTexto("Codigo: ");
    if (estoque.buscar(codigo) == nullptr) {
        std::cout << "Produto nao encontrado.\n";
        return;
    }
    int quantidade = 0;
    if (!lerQuantidade("Nova quantidade: ", quantidade)) {
        std::cout << "Quantidade invalida. Informe um inteiro nao negativo.\n";
        return;
    }
    estoque.atualizarQuantidade(codigo, quantidade);
    std::cout << "Quantidade atualizada.\n";
    salvar(estoque);
}

void editar(ArvoreRubroNegra& estoque) {
    const Produto* atual = estoque.buscar(lerTexto("Codigo: "));
    if (atual == nullptr) {
        std::cout << "Produto nao encontrado.\n";
        return;
    }
    Produto editado = *atual;
    imprimirCabecalho();
    imprimirProduto(editado);
    std::cout << "Deixe em branco para manter o valor atual.\n";

    const std::string nome = lerTexto("Nome: ");
    if (!nome.empty()) {
        editado.nome = nome;
    }
    const std::string categoria = lerTexto("Categoria: ");
    if (!categoria.empty()) {
        editado.categoria = categoria;
    }
    const std::string quantidade = lerTexto("Quantidade: ");
    if (!quantidade.empty() &&
        (!converterInteiro(quantidade, editado.quantidade) || editado.quantidade < 0)) {
        std::cout << "Quantidade invalida. Nenhuma alteracao foi feita.\n";
        return;
    }
    const std::string preco = lerTexto("Preco: ");
    if (!preco.empty() && !converterPreco(preco, editado.preco)) {
        std::cout << "Preco invalido. Nenhuma alteracao foi feita.\n";
        return;
    }
    estoque.atualizar(editado);
    std::cout << "Produto atualizado.\n";
    salvar(estoque);
}

void remover(ArvoreRubroNegra& estoque) {
    if (estoque.remover(lerTexto("Codigo: "))) {
        std::cout << "Produto removido.\n";
        salvar(estoque);
    } else {
        std::cout << "Produto nao encontrado.\n";
    }
}

void importar(ArvoreRubroNegra& estoque) {
    ResultadoImportacao resultado;
    if (importarCsv(lerTexto("Caminho do arquivo CSV: "), estoque, std::cout, std::cerr, &resultado) &&
        resultado.inseridos > 0) {
        salvar(estoque);
    }
}

void exportar(const ArvoreRubroNegra& estoque) {
    const std::string caminho = lerTexto("Salvar como (caminho do arquivo CSV): ");
    if (caminho.empty()) {
        std::cout << "Caminho vazio; exportacao cancelada.\n";
        return;
    }
    if (exportarCsv(caminho, estoque, std::cerr)) {
        std::cout << estoque.tamanho() << " produto(s) exportado(s) para " << caminho << ".\n";
    }
}

void mostrarArvore(const ArvoreRubroNegra& estoque) {
    std::string erro;
    const bool valida = estoque.validar(&erro);
    std::cout << "Produtos: " << estoque.tamanho()
              << " | Altura: " << estoque.altura()
              << " | Altura negra: " << estoque.alturaNegra()
              << " | Propriedades rubro-negras: " << (valida ? "OK" : "VIOLADAS (" + erro + ")")
              << "\nLegenda: (P) preto, (V) vermelho, E: filho esquerdo, D: filho direito\n\n";
    estoque.imprimirEstrutura(std::cout);
}

}  // namespace

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    ArvoreRubroNegra estoque;
    std::size_t carregados = 0;
    if (carregarEstoque(estoque, std::cerr, &carregados) && carregados > 0) {
        std::cout << carregados << " produto(s) carregado(s) de " << ARQUIVO_ESTOQUE << ".\n";
    }

    std::string opcao;
    do {
        exibirMenu();
        opcao = lerTexto("Opcao: ");
        if (opcao == "1") {
            cadastrar(estoque);
        } else if (opcao == "2") {
            consultar(estoque);
        } else if (opcao == "3") {
            listar(estoque);
        } else if (opcao == "4") {
            atualizarQuantidade(estoque);
        } else if (opcao == "5") {
            editar(estoque);
        } else if (opcao == "6") {
            remover(estoque);
        } else if (opcao == "7") {
            importar(estoque);
        } else if (opcao == "8") {
            exportar(estoque);
        } else if (opcao == "9") {
            mostrarArvore(estoque);
        } else if (opcao != "0" && !std::cin.eof()) {
            std::cout << "Opcao invalida.\n";
        }
    } while (opcao != "0" && !std::cin.eof());

    std::cout << "Programa encerrado.\n";
    return 0;
}
