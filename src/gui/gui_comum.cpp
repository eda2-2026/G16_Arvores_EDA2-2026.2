#include "gui_comum.hpp"

#include <iomanip>
#include <sstream>

const COLORREF COR_FUNDO = RGB(245, 248, 246);
const COLORREF COR_VERDE = RGB(20, 67, 55);
const COLORREF COR_TEXTO = RGB(34, 49, 43);
const COLORREF COR_SUAVE = RGB(111, 128, 119);
const COLORREF COR_DESTAQUE = RGB(37, 132, 99);

HFONT fonteApp = nullptr;

std::wstring paraWide(const std::string& texto) {
    if (texto.empty()) {
        return std::wstring();
    }
    const int tamanho = MultiByteToWideChar(CP_UTF8, 0, texto.c_str(), -1, nullptr, 0);
    if (tamanho <= 0) {
        return std::wstring(texto.begin(), texto.end());
    }
    std::wstring resultado(static_cast<std::size_t>(tamanho), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, texto.c_str(), -1, &resultado[0], tamanho);
    resultado.resize(static_cast<std::size_t>(tamanho - 1));
    return resultado;
}

std::string paraUtf8(const std::wstring& texto) {
    if (texto.empty()) {
        return std::string();
    }
    const int tamanho = WideCharToMultiByte(CP_UTF8, 0, texto.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (tamanho <= 0) {
        return std::string(texto.begin(), texto.end());
    }
    std::string resultado(static_cast<std::size_t>(tamanho), '\0');
    WideCharToMultiByte(CP_UTF8, 0, texto.c_str(), -1, &resultado[0], tamanho, nullptr, nullptr);
    resultado.resize(static_cast<std::size_t>(tamanho - 1));
    return resultado;
}

std::wstring formatarNumero(long long numero) {
    std::wostringstream fluxo;
    fluxo << numero;
    return fluxo.str();
}

std::wstring formatarPreco(double preco) {
    std::wostringstream fluxo;
    fluxo << std::fixed << std::setprecision(2) << preco;
    return fluxo.str();
}

HWND criarControle(DWORD estilo, const wchar_t* classe, const wchar_t* texto,
                   int id, int x, int y, int largura, int altura, HWND pai) {
    HWND controle = CreateWindowExW(0, classe, texto, WS_CHILD | WS_VISIBLE | estilo,
                                    x, y, largura, altura, pai,
                                    reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                                    GetModuleHandleW(nullptr), nullptr);
    SendMessageW(controle, WM_SETFONT, reinterpret_cast<WPARAM>(fonteApp), TRUE);
    return controle;
}

std::wstring lerCampo(HWND controle) {
    const int tamanho = GetWindowTextLengthW(controle);
    std::wstring texto(static_cast<std::size_t>(tamanho + 1), L'\0');
    GetWindowTextW(controle, &texto[0], tamanho + 1);
    texto.resize(static_cast<std::size_t>(tamanho));
    return texto;
}
