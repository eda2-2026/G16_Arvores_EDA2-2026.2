#include "utilitarios.hpp"

#include <cmath>
#include <exception>

std::string aparar(const std::string& texto) {
    const std::string espacos = " \t\r\n";
    const std::size_t inicio = texto.find_first_not_of(espacos);
    if (inicio == std::string::npos) {
        return "";
    }
    return texto.substr(inicio, texto.find_last_not_of(espacos) - inicio + 1);
}

bool converterInteiro(const std::string& texto, int& valor) {
    try {
        std::size_t consumidos = 0;
        const int convertido = std::stoi(texto, &consumidos);
        if (consumidos != texto.size()) {
            return false;
        }
        valor = convertido;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool converterPreco(const std::string& texto, double& valor) {
    try {
        std::size_t consumidos = 0;
        const double convertido = std::stod(texto, &consumidos);
        if (consumidos != texto.size() || !std::isfinite(convertido) || convertido < 0.0) {
            return false;
        }
        valor = convertido;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}
