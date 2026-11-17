#include "janela_arvore.hpp"

#include <algorithm>
#include <sstream>

namespace {

const wchar_t* const CLASSE_ARVORE = L"EstoqueJanelaArvore";
const int ID_TEXTO_ARVORE = 300;

HWND janelaArvore = nullptr;
HWND textoArvore = nullptr;
HFONT fonteMonoespacada = nullptr;

std::wstring descreverArvore(const ArvoreRubroNegra& estoque) {
    std::string erro;
    const bool valida = estoque.validar(&erro);
    std::ostringstream texto;
    texto << "Produtos: " << estoque.tamanho()
          << "   |   Altura: " << estoque.altura()
          << "   |   Altura negra: " << estoque.alturaNegra()
          << "   |   Propriedades rubro-negras: " << (valida ? "OK" : "VIOLADAS (" + erro + ")")
          << "\nLegenda: (P) preto, (V) vermelho, E: filho esquerdo, D: filho direito\n\n";
    estoque.imprimirEstrutura(texto);

    // O controle EDIT multilinha exige \r\n como quebra de linha.
    std::wstring largo = paraWide(texto.str());
    std::wstring resultado;
    resultado.reserve(largo.size() + largo.size() / 16);
    for (const wchar_t caractere : largo) {
        if (caractere == L'\n') {
            resultado += L'\r';
        }
        resultado += caractere;
    }
    return resultado;
}

LRESULT CALLBACK procedimentoArvore(HWND janela, UINT mensagem, WPARAM wParam, LPARAM lParam) {
    switch (mensagem) {
    case WM_CREATE:
        fonteMonoespacada = CreateFontW(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                        FIXED_PITCH | FF_MODERN, L"Consolas");
        textoArvore = CreateWindowExW(0, L"EDIT", L"",
                                      WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
                                          ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
                                      0, 0, 0, 0, janela,
                                      reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_TEXTO_ARVORE)),
                                      GetModuleHandleW(nullptr), nullptr);
        SendMessageW(textoArvore, WM_SETFONT, reinterpret_cast<WPARAM>(fonteMonoespacada), TRUE);
        SendMessageW(textoArvore, EM_SETLIMITTEXT, 0, 0);  // Remove o limite padrão de 32 KB.
        return 0;
    case WM_SIZE:
        MoveWindow(textoArvore, 12, 12, (std::max)(0, LOWORD(lParam) - 24),
                   (std::max)(0, HIWORD(lParam) - 24), TRUE);
        return 0;
    case WM_CTLCOLORSTATIC:
        // Controles somente leitura usam este evento; mantém o fundo branco.
        SetBkColor(reinterpret_cast<HDC>(wParam), RGB(255, 255, 255));
        SetTextColor(reinterpret_cast<HDC>(wParam), COR_TEXTO);
        return reinterpret_cast<LRESULT>(GetStockObject(WHITE_BRUSH));
    case WM_DESTROY:
        if (fonteMonoespacada != nullptr) {
            DeleteObject(fonteMonoespacada);
            fonteMonoespacada = nullptr;
        }
        janelaArvore = nullptr;
        textoArvore = nullptr;
        return 0;
    default:
        break;
    }
    return DefWindowProcW(janela, mensagem, wParam, lParam);
}

}  // namespace

bool registrarClasseArvore(HINSTANCE instancia) {
    WNDCLASSW classe = {};
    classe.lpfnWndProc = procedimentoArvore;
    classe.hInstance = instancia;
    classe.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    classe.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    classe.lpszClassName = CLASSE_ARVORE;
    return RegisterClassW(&classe) != 0;
}

void mostrarJanelaArvore(HWND pai, const ArvoreRubroNegra& estoque) {
    if (janelaArvore == nullptr) {
        janelaArvore = CreateWindowExW(0, CLASSE_ARVORE, L"Estrutura da arvore rubro-negra",
                                       WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 720, 600,
                                       pai, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (janelaArvore == nullptr) {
            MessageBoxW(pai, L"Nao foi possivel abrir a visualizacao da arvore.",
                        L"Erro", MB_OK | MB_ICONERROR);
            return;
        }
        ShowWindow(janelaArvore, SW_SHOW);
    }
    atualizarJanelaArvore(estoque);
    if (IsIconic(janelaArvore)) {
        ShowWindow(janelaArvore, SW_RESTORE);
    }
    SetForegroundWindow(janelaArvore);
}

void atualizarJanelaArvore(const ArvoreRubroNegra& estoque) {
    if (textoArvore == nullptr) {
        return;
    }
    SetWindowTextW(textoArvore, descreverArvore(estoque).c_str());
}
