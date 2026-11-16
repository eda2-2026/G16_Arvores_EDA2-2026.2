#include "persistencia.hpp"

#include <fstream>
#include <ostream>
#include <sstream>

#include "csv.hpp"

bool carregarEstoque(ArvoreRubroNegra& estoque, std::ostream& erros, std::size_t* carregados) {
    if (carregados != nullptr) {
        *carregados = 0;
    }
    {
        std::ifstream existe(ARQUIVO_ESTOQUE);
        if (!existe) {
            return true;  // Primeira execução: ainda não há estoque salvo.
        }
    }

    std::ostringstream resumo;
    ResultadoImportacao resultado;
    if (!importarCsv(ARQUIVO_ESTOQUE, estoque, resumo, erros, &resultado)) {
        std::ifstream original(ARQUIVO_ESTOQUE, std::ios::binary);
        std::ofstream copia(ARQUIVO_BACKUP, std::ios::binary | std::ios::trunc);
        if (original && copia) {
            copia << original.rdbuf();
            erros << "O arquivo '" << ARQUIVO_ESTOQUE << "' nao pode ser carregado. "
                  << "Uma copia foi salva em '" << ARQUIVO_BACKUP << "'.\n";
        }
        return false;
    }
    if (carregados != nullptr) {
        *carregados = resultado.inseridos;
    }
    return true;
}

bool salvarEstoque(const ArvoreRubroNegra& estoque, std::ostream& erros) {
    return exportarCsv(ARQUIVO_ESTOQUE, estoque, erros);
}
