#ifndef CSV_HPP
#define CSV_HPP

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

#include "arvore_rubro_negra.hpp"

// Formato: cabeçalho "codigo,nome,categoria,quantidade,preco", UTF-8,
// campos com vírgula entre aspas e aspas internas escapadas como "".

struct ResultadoImportacao {
    std::size_t inseridos = 0;
    std::size_t ignorados = 0;
};

// Separa uma linha CSV em campos. Retorna false se as aspas estiverem malformadas.
bool separarCsv(const std::string& linha, std::vector<std::string>& campos);

// Coloca o campo entre aspas quando necessário.
std::string escaparCampoCsv(const std::string& campo);

// Insere na árvore os produtos válidos do arquivo. Linhas inválidas e códigos
// duplicados são ignorados com aviso em `erros`. Retorna false em erro fatal
// (arquivo inexistente, cabeçalho inválido ou falha de leitura).
bool importarCsv(const std::string& caminho, ArvoreRubroNegra& estoque,
                 std::ostream& saida, std::ostream& erros,
                 ResultadoImportacao* resultado = nullptr);

// Grava todos os produtos, em ordem de código, no mesmo formato da importação.
bool exportarCsv(const std::string& caminho, const ArvoreRubroNegra& estoque,
                 std::ostream& erros);

#endif
