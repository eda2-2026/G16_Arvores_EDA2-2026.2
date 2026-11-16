#include "formulario.hpp"

#include "utilitarios.hpp"

namespace {

const wchar_t* const CLASSE_FORMULARIO = L"EstoqueFormulario";

enum ControleFormulario {
    ID_FORM_CODIGO = 200,
    ID_FORM_NOME,
    ID_FORM_CATEGORIA,
    ID_FORM_QUANTIDADE,
    ID_FORM_PRECO,
    ID_FORM_SALVAR,
    ID_FORM_CANCELAR
};

enum Campo { CAMPO_CODIGO, CAMPO_NOME, CAMPO_CATEGORIA, CAMPO_QUANTIDADE, CAMPO_PRECO, TOTAL_CAMPOS };

struct EstadoFormulario {
    bool aceito = false;
    ModoFormulario modo = ModoFormulario::Novo;
    Produto produto{"", "", "", 0, 0.0};
    HWND janela = nullptr;
    HWND campos[TOTAL_CAMPOS] = {};
};

EstadoFormulario estado;

std::string textoCampo(Campo campo) {
    return aparar(paraUtf8(lerCampo(estado.campos[campo])));
}

void criarCampos(HWND janela) {
    if (estado.modo == ModoFormulario::Quantidade) {
        criarControle(0, L"STATIC", L"Informe a nova quantidade em estoque:",
                      0, 28, 29, 360, 24, janela);
        estado.campos[CAMPO_QUANTIDADE] =
            criarControle(WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, L"EDIT",
                          formatarNumero(estado.produto.quantidade).c_str(),
                          ID_FORM_QUANTIDADE, 28, 62, 364, 36, janela);
        return;
    }

    const wchar_t* rotulos[] = {L"Codigo do produto", L"Nome", L"Categoria", L"Quantidade", L"Preco (R$)"};
    const std::wstring valores[] = {
        paraWide(estado.produto.codigo),
        paraWide(estado.produto.nome),
        paraWide(estado.produto.categoria),
        formatarNumero(estado.produto.quantidade),
        formatarPreco(estado.produto.preco)
    };
    const int ids[] = {ID_FORM_CODIGO, ID_FORM_NOME, ID_FORM_CATEGORIA, ID_FORM_QUANTIDADE, ID_FORM_PRECO};
    for (int i = 0; i < TOTAL_CAMPOS; ++i) {
        const int y = 22 + i * 58;
        criarControle(0, L"STATIC", rotulos[i], 0, 28, y, 360, 20, janela);
        estado.campos[i] = criarControle(WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, L"EDIT",
                                         valores[i].c_str(), ids[i], 28, y + 22, 364, 32, janela);
    }
    if (estado.modo == ModoFormulario::Editar) {
        // O código é a chave da árvore; mudá-lo exigiria remover e reinserir o nó.
        SendMessageW(estado.campos[CAMPO_CODIGO], EM_SETREADONLY, TRUE, 0);
    }
}

// Valida os campos e, se estiverem corretos, copia os valores para estado.produto.
bool validarEGravar(HWND janela) {
    int quantidade = 0;
    if (!converterInteiro(textoCampo(CAMPO_QUANTIDADE), quantidade) || quantidade < 0) {
        MessageBoxW(janela, L"Informe uma quantidade inteira igual ou maior que zero.",
                    L"Valor invalido", MB_OK | MB_ICONWARNING);
        SetFocus(estado.campos[CAMPO_QUANTIDADE]);
        return false;
    }
    if (estado.modo == ModoFormulario::Quantidade) {
        estado.produto.quantidade = quantidade;
        return true;
    }

    const std::string codigo = textoCampo(CAMPO_CODIGO);
    const std::string nome = textoCampo(CAMPO_NOME);
    const std::string categoria = textoCampo(CAMPO_CATEGORIA);
    if (codigo.empty() || nome.empty() || categoria.empty()) {
        MessageBoxW(janela, L"Preencha o codigo, o nome e a categoria do produto.",
                    L"Campos obrigatorios", MB_OK | MB_ICONWARNING);
        return false;
    }
    double preco = 0.0;
    if (!converterPreco(textoCampo(CAMPO_PRECO), preco)) {
        MessageBoxW(janela, L"Informe um preco valido, usando ponto decimal.",
                    L"Valor invalido", MB_OK | MB_ICONWARNING);
        SetFocus(estado.campos[CAMPO_PRECO]);
        return false;
    }
    if (estado.modo == ModoFormulario::Novo) {
        estado.produto.codigo = codigo;
    }
    estado.produto.nome = nome;
    estado.produto.categoria = categoria;
    estado.produto.quantidade = quantidade;
    estado.produto.preco = preco;
    return true;
}

void desenharBotaoFormulario(const DRAWITEMSTRUCT* item) {
    const bool primario = item->CtlID == ID_FORM_SALVAR;
    COLORREF cor = primario ? COR_DESTAQUE : RGB(239, 244, 241);
    if (item->itemState & ODS_SELECTED) {
        cor = RGB(GetRValue(cor) * 9 / 10, GetGValue(cor) * 9 / 10, GetBValue(cor) * 9 / 10);
    }
    HBRUSH pincel = CreateSolidBrush(cor);
    FillRect(item->hDC, &item->rcItem, pincel);
    DeleteObject(pincel);
    SetBkMode(item->hDC, TRANSPARENT);
    SetTextColor(item->hDC, primario ? RGB(255, 255, 255) : COR_TEXTO);
    wchar_t texto[32] = {};
    GetWindowTextW(item->hwndItem, texto, 32);
    RECT area = item->rcItem;
    DrawTextW(item->hDC, texto, -1, &area, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    if (item->itemState & ODS_FOCUS) {
        DrawFocusRect(item->hDC, &area);
    }
}

// Reabilita a janela principal antes de destruir o formulário para que o foco volte a ela.
void fecharFormulario(HWND janela) {
    EnableWindow(GetWindow(janela, GW_OWNER), TRUE);
    DestroyWindow(janela);
}

LRESULT CALLBACK procedimentoFormulario(HWND janela, UINT mensagem, WPARAM wParam, LPARAM lParam) {
    switch (mensagem) {
    case WM_CREATE: {
        criarCampos(janela);
        const int botoesY = estado.modo == ModoFormulario::Quantidade ? 116 : 341;
        criarControle(BS_OWNERDRAW | WS_TABSTOP, L"BUTTON", L"Salvar", ID_FORM_SALVAR,
                      178, botoesY, 102, 38, janela);
        criarControle(BS_OWNERDRAW | WS_TABSTOP, L"BUTTON", L"Cancelar", ID_FORM_CANCELAR,
                      290, botoesY, 102, 38, janela);
        return 0;
    }
    case WM_COMMAND: {
        const int id = LOWORD(wParam);
        // IDOK/IDCANCEL chegam quando o usuário pressiona Enter/Esc.
        if (id == ID_FORM_CANCELAR || id == IDCANCEL) {
            fecharFormulario(janela);
            return 0;
        }
        if (id == ID_FORM_SALVAR || id == IDOK) {
            if (validarEGravar(janela)) {
                estado.aceito = true;
                fecharFormulario(janela);
            }
            return 0;
        }
        break;
    }
    case WM_DRAWITEM:
        desenharBotaoFormulario(reinterpret_cast<const DRAWITEMSTRUCT*>(lParam));
        return TRUE;
    case WM_CLOSE:
        fecharFormulario(janela);
        return 0;
    default:
        break;
    }
    return DefWindowProcW(janela, mensagem, wParam, lParam);
}

}  // namespace

bool registrarClasseFormulario(HINSTANCE instancia) {
    WNDCLASSW classe = {};
    classe.lpfnWndProc = procedimentoFormulario;
    classe.hInstance = instancia;
    classe.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    classe.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    classe.lpszClassName = CLASSE_FORMULARIO;
    return RegisterClassW(&classe) != 0;
}

bool abrirFormulario(HWND pai, const wchar_t* titulo, ModoFormulario modo, Produto& produto) {
    estado = EstadoFormulario{};
    estado.modo = modo;
    estado.produto = produto;
    const bool soQuantidade = modo == ModoFormulario::Quantidade;

    // Centraliza o formulário sobre a janela principal.
    const int largura = 440;
    const int altura = soQuantidade ? 200 : 430;
    RECT areaPai = {};
    GetWindowRect(pai, &areaPai);
    const int x = areaPai.left + ((areaPai.right - areaPai.left) - largura) / 2;
    const int y = areaPai.top + ((areaPai.bottom - areaPai.top) - altura) / 2;

    estado.janela = CreateWindowExW(WS_EX_DLGMODALFRAME, CLASSE_FORMULARIO, titulo,
                                    WS_CAPTION | WS_SYSMENU, x, y, largura, altura,
                                    pai, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (estado.janela == nullptr) {
        MessageBoxW(pai, L"Nao foi possivel abrir o formulario.", L"Erro", MB_OK | MB_ICONERROR);
        return false;
    }
    EnableWindow(pai, FALSE);
    ShowWindow(estado.janela, SW_SHOW);
    SetForegroundWindow(estado.janela);
    HWND primeiroCampo = estado.campos[CAMPO_CODIGO];
    if (soQuantidade) {
        primeiroCampo = estado.campos[CAMPO_QUANTIDADE];
    } else if (modo == ModoFormulario::Editar) {
        primeiroCampo = estado.campos[CAMPO_NOME];
    }
    SetFocus(primeiroCampo);
    SendMessageW(primeiroCampo, EM_SETSEL, 0, -1);

    // Laço de mensagens próprio: torna o formulário modal.
    MSG mensagem;
    int resultado = 0;
    while (IsWindow(estado.janela) && (resultado = GetMessageW(&mensagem, nullptr, 0, 0)) > 0) {
        if (!IsDialogMessageW(estado.janela, &mensagem)) {
            TranslateMessage(&mensagem);
            DispatchMessageW(&mensagem);
        }
    }
    if (resultado == 0) {
        PostQuitMessage(static_cast<int>(mensagem.wParam));  // Repassa o WM_QUIT recebido.
    }
    if (IsWindow(estado.janela)) {
        DestroyWindow(estado.janela);
    }
    EnableWindow(pai, TRUE);
    SetForegroundWindow(pai);
    if (estado.aceito) {
        produto = estado.produto;
    }
    return estado.aceito;
}
