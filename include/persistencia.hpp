#ifndef PERSISTENCIA_HPP
#define PERSISTENCIA_HPP

#include <cstddef>
#include <iosfwd>

#include "arvore_rubro_negra.hpp"

// Arquivo onde o estoque é salvo automaticamente, na pasta em que o programa roda.
inline constexpr const char* ARQUIVO_ESTOQUE = "estoque.csv";
inline constexpr const char* ARQUIVO_BACKUP = "estoque.csv.bak";

// Carrega o estoque salvo. Se o arquivo não existir, retorna true e não carrega nada.
// Se estiver corrompido, faz uma cópia em ARQUIVO_BACKUP antes de ser sobrescrito
// e retorna false. Avisos e erros vão para `erros`.
bool carregarEstoque(ArvoreRubroNegra& estoque, std::ostream& erros,
                     std::size_t* carregados = nullptr);

// Salva o estoque inteiro em ARQUIVO_ESTOQUE.
bool salvarEstoque(const ArvoreRubroNegra& estoque, std::ostream& erros);

#endif
