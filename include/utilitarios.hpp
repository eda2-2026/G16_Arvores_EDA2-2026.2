#ifndef UTILITARIOS_HPP
#define UTILITARIOS_HPP

#include <string>

// Remove espaços, tabulações e quebras de linha do início e do fim do texto.
std::string aparar(const std::string& texto);

// Converte o texto inteiro para int. Retorna false se houver sobras ou estouro.
bool converterInteiro(const std::string& texto, int& valor);

// Converte um preço não negativo e finito (ponto como separador decimal).
bool converterPreco(const std::string& texto, double& valor);

#endif
