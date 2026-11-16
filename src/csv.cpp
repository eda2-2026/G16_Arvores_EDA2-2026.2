#include "csv.hpp"

#include <cctype>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <ostream>

#include "utilitarios.hpp"

bool separarCsv(const std::string& linha, std::vector<std::string>& campos) {
    campos.clear();
    std::string campo;
    bool entreAspas = false;
    bool aspasFechadas = false;
    for (std::size_t i = 0; i < linha.size(); ++i) {
        const char caractere = linha[i];
        if (entreAspas) {
            if (caractere == '"') {
                if (i + 1 < linha.size() && linha[i + 1] == '"') {
                    campo += '"';
                    ++i;
                } else {
                    entreAspas = false;
                    aspasFechadas = true;
                }
            } else {
                campo += caractere;
            }
        } else if (caractere == ',') {
            campos.push_back(aparar(campo));
            campo.clear();
            aspasFechadas = false;
        } else if (caractere == '"' && campo.empty() && !aspasFechadas) {
            entreAspas = true;
        } else if (aspasFechadas) {
            if (!std::isspace(static_cast<unsigned char>(caractere))) {
                return false;
            }
        } else {
            campo += caractere;
        }
    }
    if (entreAspas) {
        return false;
    }
    campos.push_back(aparar(campo));
    return true;
}

std::string escaparCampoCsv(const std::string& campo) {
    if (campo.find_first_of(",\"\r\n") == std::string::npos) {
        return campo;
    }
    std::string resultado = "\"";
    for (const char caractere : campo) {
        if (caractere == '"') {
            resultado += '"';
        }
        resultado += caractere;
    }
    resultado += '"';
    return resultado;
}

bool importarCsv(const std::string& caminho, ArvoreRubroNegra& estoque,
                 std::ostream& saida, std::ostream& erros, ResultadoImportacao* resultado) {
    std::ifstream arquivo(caminho);
    if (!arquivo) {
        erros << "Erro: nao foi possivel abrir o arquivo '" << caminho << "'.\n";
        return false;
    }

    std::string linha;
    std::vector<std::string> campos;
    if (!std::getline(arquivo, linha) || !separarCsv(linha, campos)) {
        erros << "Erro: arquivo vazio ou cabecalho CSV invalido.\n";
        return false;
    }
    // Remove o BOM do UTF-8, gravado por editores como o Excel e o Bloco de Notas.
    if (!campos.empty() && campos[0].size() >= 3 &&
        static_cast<unsigned char>(campos[0][0]) == 0xEF &&
        static_cast<unsigned char>(campos[0][1]) == 0xBB &&
        static_cast<unsigned char>(campos[0][2]) == 0xBF) {
        campos[0].erase(0, 3);
    }
    const std::vector<std::string> cabecalhoEsperado = {
        "codigo", "nome", "categoria", "quantidade", "preco"
    };
    if (campos != cabecalhoEsperado) {
        erros << "Erro: cabecalho esperado: codigo,nome,categoria,quantidade,preco\n";
        return false;
    }

    std::size_t linhaAtual = 1;
    ResultadoImportacao contagem;
    while (std::getline(arquivo, linha)) {
        ++linhaAtual;
        if (aparar(linha).empty()) {
            continue;
        }
        int quantidade = 0;
        double preco = 0.0;
        if (!separarCsv(linha, campos) || campos.size() != 5 ||
            campos[0].empty() || campos[1].empty() || campos[2].empty() ||
            !converterInteiro(campos[3], quantidade) || quantidade < 0 ||
            !converterPreco(campos[4], preco)) {
            erros << "Aviso: linha " << linhaAtual << " invalida; ignorada.\n";
            ++contagem.ignorados;
            continue;
        }
        if (!estoque.inserir(Produto{campos[0], campos[1], campos[2], quantidade, preco})) {
            erros << "Aviso: codigo duplicado na linha " << linhaAtual << " ("
                  << campos[0] << "); ignorado.\n";
            ++contagem.ignorados;
            continue;
        }
        ++contagem.inseridos;
    }
    if (arquivo.bad()) {
        erros << "Erro de leitura durante a importacao do arquivo.\n";
        return false;
    }
    if (resultado != nullptr) {
        *resultado = contagem;
    }
    saida << "Importacao concluida: " << contagem.inseridos << " produto(s) inserido(s), "
          << contagem.ignorados << " linha(s) ignorada(s).\n";
    return true;
}

bool exportarCsv(const std::string& caminho, const ArvoreRubroNegra& estoque,
                 std::ostream& erros) {
    // Grava primeiro num arquivo temporário para não corromper o original se algo falhar.
    const std::string temporario = caminho + ".tmp";
    {
        std::ofstream arquivo(temporario, std::ios::binary | std::ios::trunc);
        if (!arquivo) {
            erros << "Erro: nao foi possivel criar o arquivo '" << temporario << "'.\n";
            return false;
        }
        arquivo << "codigo,nome,categoria,quantidade,preco\n";
        arquivo << std::fixed << std::setprecision(2);
        for (const Produto& produto : estoque.produtosEmOrdem()) {
            arquivo << escaparCampoCsv(produto.codigo) << ','
                    << escaparCampoCsv(produto.nome) << ','
                    << escaparCampoCsv(produto.categoria) << ','
                    << produto.quantidade << ','
                    << produto.preco << '\n';
        }
        arquivo.flush();
        if (!arquivo) {
            erros << "Erro: falha ao gravar o arquivo '" << temporario << "'.\n";
            std::remove(temporario.c_str());
            return false;
        }
    }
    std::remove(caminho.c_str());
    if (std::rename(temporario.c_str(), caminho.c_str()) != 0) {
        erros << "Erro: nao foi possivel substituir o arquivo '" << caminho << "'.\n";
        return false;
    }
    return true;
}
