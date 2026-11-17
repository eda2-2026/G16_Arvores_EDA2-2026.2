// Interface gráfica (Win32) do gerenciador de estoque.

#include "gui_comum.hpp"

#ifndef CB_SETMINVISIBLE
#define CB_SETMINVISIBLE 0x1701
#endif

#include <algorithm>
#include <cwctype>
#include <sstream>
#include <vector>

#include "arvore_rubro_negra.hpp"
#include "csv.hpp"
#include "formulario.hpp"
#include "janela_arvore.hpp"
#include "persistencia.hpp"

namespace {

enum Controle {
    ID_BUSCA = 100,
    ID_TABELA,
    ID_NOVO,
    ID_EDITAR,
    ID_ATUALIZAR,
    ID_REMOVER,
    ID_IMPORTAR,
    ID_EXPORTAR,
    ID_ARVORE,
    ID_ORDENAR,
    ID_CATEGORIA,
    ID_ROTULO_ORDENAR,
    ID_ROTULO_CATEGORIA
};

struct Botao {
    int id;
    const wchar_t* texto;
    int largura;
};

const Botao BOTOES[] = {
    {ID_NOVO, L"+  Novo produto", 140},
    {ID_EDITAR, L"Editar", 100},
    {ID_ATUALIZAR, L"Atualizar estoque", 170},
    {ID_REMOVER, L"Remover", 105},
    {ID_IMPORTAR, L"Importar CSV", 125},
    {ID_EXPORTAR, L"Exportar CSV", 125},
    {ID_ARVORE, L"Ver arvore", 120},
};

// Layout (em pixels).
const int MARGEM = 34;
const int ESPACO_BOTOES = 10;
const int TOPO_CARTOES = 190;
const int BASE_CARTOES = 288;
const int TOPO_TITULO = 304;
const int TOPO_BUSCA = 300;
const int ALTURA_BUSCA = 32;
const int ALTURA_COMBO_ORDENAR = 250;
const int ALTURA_COMBO_CATEGORIA = 300;
const int TOPO_BOTOES = 346;
const int ALTURA_BOTOES = 38;
const int TOPO_TABELA = 402;

ArvoreRubroNegra* estoqueApp = nullptr;
HWND janelaPrincipal = nullptr;
HWND buscaApp = nullptr;
HWND tabelaApp = nullptr;
HWND ordenarApp = nullptr;
HWND categoriaApp = nullptr;
bool categoriasDesatualizadas = true;

RECT retanguloBusca(int largura) {
    const int larguraBusca = (std::min)(360, (std::max)(220, largura - MARGEM * 2 - 300));
    return RECT{largura - MARGEM - larguraBusca, TOPO_BUSCA, largura - MARGEM, TOPO_BUSCA + ALTURA_BUSCA};
}

void desenharTexto(HDC dc, const wchar_t* texto, int x, int y, COLORREF cor,
                   int tamanho = 10, int peso = FW_NORMAL) {
    HFONT fonte = CreateFontW(-MulDiv(tamanho, GetDeviceCaps(dc, LOGPIXELSY), 72),
                              0, 0, 0, peso, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                              OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                              DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HGDIOBJ anterior = SelectObject(dc, fonte);
    SetTextColor(dc, cor);
    SetBkMode(dc, TRANSPARENT);
    TextOutW(dc, x, y, texto, static_cast<int>(wcslen(texto)));
    SelectObject(dc, anterior);
    DeleteObject(fonte);
}

void desenharCartao(HDC dc, RECT retangulo, const wchar_t* rotulo,
                    const wchar_t* valor, const wchar_t* descricao) {
    HBRUSH branco = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(dc, &retangulo, branco);
    DeleteObject(branco);
    desenharTexto(dc, rotulo, retangulo.left + 18, retangulo.top + 13, COR_SUAVE, 9, FW_SEMIBOLD);
    desenharTexto(dc, valor, retangulo.left + 18, retangulo.top + 33, COR_TEXTO, 21, FW_BOLD);
    desenharTexto(dc, descricao, retangulo.left + 18, retangulo.top + 67, COR_SUAVE, 9);
}

void ajustarLayout(int largura, int altura) {
    if (buscaApp == nullptr || tabelaApp == nullptr) {
        return;
    }
    int x = MARGEM;
    for (const Botao& botao : BOTOES) {
        MoveWindow(GetDlgItem(janelaPrincipal, botao.id), x, TOPO_BOTOES, botao.largura, ALTURA_BOTOES, TRUE);
        x += botao.largura + ESPACO_BOTOES;
    }
    MoveWindow(GetDlgItem(janelaPrincipal, ID_ROTULO_ORDENAR), MARGEM + 86, TOPO_BUSCA + 6, 60, 20, TRUE);
    MoveWindow(ordenarApp, MARGEM + 146, TOPO_BUSCA, 170, ALTURA_COMBO_ORDENAR, TRUE);
    MoveWindow(GetDlgItem(janelaPrincipal, ID_ROTULO_CATEGORIA), MARGEM + 331,
               TOPO_BUSCA + 6, 60, 20, TRUE);
    MoveWindow(categoriaApp, MARGEM + 391, TOPO_BUSCA, 180, ALTURA_COMBO_CATEGORIA, TRUE);
    const RECT busca = retanguloBusca(largura);
    MoveWindow(buscaApp, busca.left, busca.top, busca.right - busca.left, busca.bottom - busca.top, TRUE);
    MoveWindow(tabelaApp, MARGEM, TOPO_TABELA, (std::max)(500, largura - MARGEM * 2),
               (std::max)(120, altura - TOPO_TABELA - 24), TRUE);
    InvalidateRect(janelaPrincipal, nullptr, TRUE);
}

void salvarAlteracoes() {
    std::ostringstream erros;
    if (!salvarEstoque(*estoqueApp, erros)) {
        const std::wstring mensagem = L"As alteracoes nao foram salvas em estoque.csv.\n\n" + paraWide(erros.str());
        MessageBoxW(janelaPrincipal, mensagem.c_str(), L"Falha ao salvar", MB_OK | MB_ICONERROR);
    }
}

std::wstring normalizarTexto(std::wstring texto) {
    std::transform(texto.begin(), texto.end(), texto.begin(), std::towlower);
    return texto;
}

std::wstring textoSelecionado(HWND controle) {
    const LRESULT indice = SendMessageW(controle, CB_GETCURSEL, 0, 0);
    if (indice == CB_ERR) {
        return std::wstring();
    }
    const LRESULT tamanho = SendMessageW(controle, CB_GETLBTEXTLEN, indice, 0);
    if (tamanho == CB_ERR) {
        return std::wstring();
    }
    std::wstring texto(static_cast<std::size_t>(tamanho + 1), L'\0');
    SendMessageW(controle, CB_GETLBTEXT, indice, reinterpret_cast<LPARAM>(&texto[0]));
    texto.resize(static_cast<std::size_t>(tamanho));
    return texto;
}

void atualizarCategorias() {
    const int indiceAnterior = static_cast<int>(SendMessageW(categoriaApp, CB_GETCURSEL, 0, 0));
    const std::wstring categoriaAnterior = indiceAnterior > 0 ? textoSelecionado(categoriaApp) : L"";
    std::vector<std::wstring> categorias;
    for (const Produto& produto : estoqueApp->produtosEmOrdem()) {
        categorias.push_back(paraWide(produto.categoria));
    }
    std::sort(categorias.begin(), categorias.end(), [](const std::wstring& a, const std::wstring& b) {
        const std::wstring chaveA = normalizarTexto(a);
        const std::wstring chaveB = normalizarTexto(b);
        return chaveA == chaveB ? a < b : chaveA < chaveB;
    });
    categorias.erase(std::unique(categorias.begin(), categorias.end(),
                                 [](const std::wstring& a, const std::wstring& b) {
                                     return normalizarTexto(a) == normalizarTexto(b);
                                 }),
                     categorias.end());

    SendMessageW(categoriaApp, CB_RESETCONTENT, 0, 0);
    SendMessageW(categoriaApp, CB_ADDSTRING, 0,
                 reinterpret_cast<LPARAM>(L"Todas as categorias"));
    int indiceSelecionado = 0;
    for (std::size_t i = 0; i < categorias.size(); ++i) {
        SendMessageW(categoriaApp, CB_ADDSTRING, 0,
                     reinterpret_cast<LPARAM>(categorias[i].c_str()));
        if (indiceAnterior > 0 &&
            normalizarTexto(categorias[i]) == normalizarTexto(categoriaAnterior)) {
            indiceSelecionado = static_cast<int>(i + 1);
        }
    }
    SendMessageW(categoriaApp, CB_SETMINVISIBLE, 8, 0);
    SendMessageW(categoriaApp, CB_SETCURSEL, indiceSelecionado, 0);
    categoriasDesatualizadas = false;
}

// Recarrega a tabela aplicando busca, categoria e ordenação. Se `selecionar`
// for informado, a linha com esse código fica selecionada e visível.
void atualizarTabela(const std::string& selecionar = std::string()) {
    if (tabelaApp == nullptr || ordenarApp == nullptr || categoriaApp == nullptr) {
        return;
    }
    if (categoriasDesatualizadas) {
        atualizarCategorias();
    }
    SendMessageW(tabelaApp, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(tabelaApp);
    const std::wstring termo = normalizarTexto(lerCampo(buscaApp));
    const int indiceCategoria = static_cast<int>(SendMessageW(categoriaApp, CB_GETCURSEL, 0, 0));
    const std::wstring categoriaFiltro = indiceCategoria > 0
        ? normalizarTexto(textoSelecionado(categoriaApp))
        : std::wstring();
    std::vector<Produto> produtos = estoqueApp->produtosEmOrdem();
    produtos.erase(std::remove_if(produtos.begin(), produtos.end(),
                                  [&termo, &categoriaFiltro](const Produto& produto) {
        const std::wstring campos[] = {
            paraWide(produto.codigo), paraWide(produto.nome), paraWide(produto.categoria),
            formatarNumero(produto.quantidade), L"R$ " + formatarPreco(produto.preco)
        };
        std::wstring textoPesquisa = campos[0] + L" " + campos[1] + L" " + campos[2];
        textoPesquisa = normalizarTexto(textoPesquisa);
        const bool naoCorrespondeBusca = !termo.empty() && textoPesquisa.find(termo) == std::wstring::npos;
        const bool naoCorrespondeCategoria = !categoriaFiltro.empty() &&
            normalizarTexto(campos[2]) != categoriaFiltro;
        return naoCorrespondeBusca || naoCorrespondeCategoria;
    }), produtos.end());

    const int ordenacao = static_cast<int>(SendMessageW(ordenarApp, CB_GETCURSEL, 0, 0));
    if (ordenacao >= 1 && ordenacao <= 4) {
        std::stable_sort(produtos.begin(), produtos.end(), [ordenacao](const Produto& a, const Produto& b) {
            const std::wstring nomeA = normalizarTexto(paraWide(a.nome));
            const std::wstring nomeB = normalizarTexto(paraWide(b.nome));
            if (ordenacao == 1 || ordenacao == 2) {
                if (a.quantidade != b.quantidade) {
                    return ordenacao == 1 ? a.quantidade < b.quantidade : a.quantidade > b.quantidade;
                }
                return nomeA == nomeB ? a.codigo < b.codigo : nomeA < nomeB;
            }
            if (nomeA == nomeB) {
                return a.codigo < b.codigo;
            }
            return ordenacao == 3 ? nomeA < nomeB : nomeA > nomeB;
        });
    }

    for (const Produto& produto : produtos) {
        const std::wstring campos[] = {
            paraWide(produto.codigo), paraWide(produto.nome), paraWide(produto.categoria),
            formatarNumero(produto.quantidade), L"R$ " + formatarPreco(produto.preco)
        };
        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = ListView_GetItemCount(tabelaApp);
        item.pszText = const_cast<LPWSTR>(campos[0].c_str());
        const int indice = ListView_InsertItem(tabelaApp, &item);
        for (int coluna = 1; coluna < 5; ++coluna) {
            ListView_SetItemText(tabelaApp, indice, coluna,
                                 const_cast<LPWSTR>(campos[coluna].c_str()));
        }
        if (!selecionar.empty() && produto.codigo == selecionar) {
            ListView_SetItemState(tabelaApp, indice, LVIS_SELECTED | LVIS_FOCUSED,
                                  LVIS_SELECTED | LVIS_FOCUSED);
            ListView_EnsureVisible(tabelaApp, indice, FALSE);
        }
    }
    SendMessageW(tabelaApp, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(tabelaApp, nullptr, TRUE);
    InvalidateRect(janelaPrincipal, nullptr, TRUE);
    atualizarJanelaArvore(*estoqueApp);
}

std::string codigoSelecionado() {
    const int indice = ListView_GetNextItem(tabelaApp, -1, LVNI_SELECTED);
    if (indice < 0) {
        return std::string();
    }
    wchar_t codigo[256] = {};
    ListView_GetItemText(tabelaApp, indice, 0, codigo, 256);
    return paraUtf8(codigo);
}

// Retorna o produto selecionado na tabela ou nullptr (avisando o usuário).
const Produto* produtoSelecionado() {
    const std::string codigo = codigoSelecionado();
    const Produto* produto = codigo.empty() ? nullptr : estoqueApp->buscar(codigo);
    if (produto == nullptr) {
        MessageBoxW(janelaPrincipal, L"Selecione um produto na tabela primeiro.",
                    L"Nenhum produto selecionado", MB_OK | MB_ICONINFORMATION);
    }
    return produto;
}

void adicionarProduto() {
    Produto novo = {"", "", "", 0, 0.0};
    if (!abrirFormulario(janelaPrincipal, L"Adicionar produto", ModoFormulario::Novo, novo)) {
        return;
    }
    if (!estoqueApp->inserir(novo)) {
        MessageBoxW(janelaPrincipal, L"Ja existe um produto com esse codigo.",
                    L"Codigo duplicado", MB_OK | MB_ICONWARNING);
        return;
    }
    categoriasDesatualizadas = true;
    salvarAlteracoes();
    atualizarTabela(novo.codigo);
}

void editarProduto() {
    const Produto* atual = produtoSelecionado();
    if (atual == nullptr) {
        return;
    }
    Produto editado = *atual;
    if (abrirFormulario(janelaPrincipal, L"Editar produto", ModoFormulario::Editar, editado)) {
        estoqueApp->atualizar(editado);
        categoriasDesatualizadas = true;
        salvarAlteracoes();
        atualizarTabela(editado.codigo);
    }
}

void atualizarQuantidade() {
    const Produto* atual = produtoSelecionado();
    if (atual == nullptr) {
        return;
    }
    Produto editado = *atual;
    if (abrirFormulario(janelaPrincipal, L"Atualizar estoque", ModoFormulario::Quantidade, editado)) {
        estoqueApp->atualizar(editado);
        salvarAlteracoes();
        atualizarTabela(editado.codigo);
    }
}

void removerProduto() {
    const Produto* produto = produtoSelecionado();
    if (produto == nullptr) {
        return;
    }
    const std::string codigo = produto->codigo;
    const std::wstring pergunta = L"Remover \"" + paraWide(produto->nome) + L"\" do estoque?";
    if (MessageBoxW(janelaPrincipal, pergunta.c_str(), L"Confirmar remocao",
                    MB_YESNO | MB_ICONQUESTION) == IDYES) {
        estoqueApp->remover(codigo);
        categoriasDesatualizadas = true;
        salvarAlteracoes();
        atualizarTabela();
    }
}

void importarArquivo() {
    wchar_t caminho[MAX_PATH] = {};
    OPENFILENAMEW dialogo = {};
    dialogo.lStructSize = sizeof(dialogo);
    dialogo.hwndOwner = janelaPrincipal;
    dialogo.lpstrFilter = L"Arquivos CSV (*.csv)\0*.csv\0Todos os arquivos (*.*)\0*.*\0";
    dialogo.lpstrFile = caminho;
    dialogo.nMaxFile = MAX_PATH;
    dialogo.lpstrTitle = L"Importar produtos do arquivo CSV";
    dialogo.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameW(&dialogo)) {
        return;
    }
    std::ostringstream saida;
    std::ostringstream erros;
    ResultadoImportacao resultado;
    if (!importarCsv(paraUtf8(caminho), *estoqueApp, saida, erros, &resultado)) {
        MessageBoxW(janelaPrincipal, paraWide(erros.str()).c_str(), L"Falha ao importar CSV",
                    MB_OK | MB_ICONERROR);
        return;
    }
    if (resultado.inseridos > 0) {
        salvarAlteracoes();
    }
    categoriasDesatualizadas = true;
    atualizarTabela();
    const std::string avisos = erros.str();
    const std::string mensagem = saida.str() + (avisos.empty() ? std::string() : "\n\nAvisos:\n" + avisos);
    MessageBoxW(janelaPrincipal, paraWide(mensagem).c_str(), L"Importacao concluida",
                MB_OK | (avisos.empty() ? MB_ICONINFORMATION : MB_ICONWARNING));
}

void exportarArquivo() {
    wchar_t caminho[MAX_PATH] = L"estoque_exportado.csv";
    OPENFILENAMEW dialogo = {};
    dialogo.lStructSize = sizeof(dialogo);
    dialogo.hwndOwner = janelaPrincipal;
    dialogo.lpstrFilter = L"Arquivos CSV (*.csv)\0*.csv\0";
    dialogo.lpstrFile = caminho;
    dialogo.nMaxFile = MAX_PATH;
    dialogo.lpstrDefExt = L"csv";
    dialogo.lpstrTitle = L"Exportar estoque para CSV";
    dialogo.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
    if (!GetSaveFileNameW(&dialogo)) {
        return;
    }
    std::ostringstream erros;
    if (!exportarCsv(paraUtf8(caminho), *estoqueApp, erros)) {
        MessageBoxW(janelaPrincipal, paraWide(erros.str()).c_str(), L"Falha ao exportar CSV",
                    MB_OK | MB_ICONERROR);
        return;
    }
    const std::wstring mensagem = formatarNumero(static_cast<long long>(estoqueApp->tamanho())) +
                                  L" produto(s) exportado(s) para:\n" + caminho;
    MessageBoxW(janelaPrincipal, mensagem.c_str(), L"Exportacao concluida", MB_OK | MB_ICONINFORMATION);
}

void desenharBotao(const DRAWITEMSTRUCT* item) {
    COLORREF cor = RGB(232, 239, 235);
    COLORREF texto = COR_TEXTO;
    if (item->CtlID == ID_NOVO) {
        cor = COR_DESTAQUE;
        texto = RGB(255, 255, 255);
    } else if (item->CtlID == ID_REMOVER) {
        cor = RGB(255, 241, 238);
        texto = RGB(173, 72, 60);
    } else if (item->CtlID == ID_ARVORE) {
        cor = COR_VERDE;
        texto = RGB(255, 255, 255);
    }
    if (item->itemState & ODS_SELECTED) {
        cor = RGB(GetRValue(cor) * 9 / 10, GetGValue(cor) * 9 / 10, GetBValue(cor) * 9 / 10);
    }
    HBRUSH pincel = CreateSolidBrush(cor);
    HPEN caneta = CreatePen(PS_SOLID, 1, cor);
    HGDIOBJ canetaAnterior = SelectObject(item->hDC, caneta);
    HGDIOBJ pincelAnterior = SelectObject(item->hDC, pincel);
    RoundRect(item->hDC, item->rcItem.left, item->rcItem.top, item->rcItem.right, item->rcItem.bottom, 12, 12);
    SelectObject(item->hDC, pincelAnterior);
    SelectObject(item->hDC, canetaAnterior);
    SetBkMode(item->hDC, TRANSPARENT);
    SetTextColor(item->hDC, texto);
    wchar_t titulo[64] = {};
    GetWindowTextW(item->hwndItem, titulo, 64);
    RECT area = item->rcItem;
    DrawTextW(item->hDC, titulo, -1, &area, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(pincel);
    DeleteObject(caneta);
}

void pintarJanela(HWND janela) {
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(janela, &ps);
    RECT area;
    GetClientRect(janela, &area);
    HBRUSH fundo = CreateSolidBrush(COR_FUNDO);
    FillRect(dc, &area, fundo);
    DeleteObject(fundo);

    RECT cabecalho = {0, 0, area.right, 165};
    HBRUSH verde = CreateSolidBrush(COR_VERDE);
    FillRect(dc, &cabecalho, verde);
    DeleteObject(verde);
    desenharTexto(dc, L"PAINEL DE ESTOQUE", 36, 22, RGB(181, 221, 201), 9, FW_BOLD);
    desenharTexto(dc, L"Controle de Estoque fácil.", 34, 48, RGB(255, 255, 255), 24, FW_BOLD);
    desenharTexto(dc, L"Gerenciador de produtos By Bryan e Pablo.", 36, 91, RGB(218, 234, 224), 10);

    long long unidades = 0;
    int poucos = 0;
    for (const Produto& produto : estoqueApp->produtosEmOrdem()) {
        unidades += produto.quantidade;
        if (produto.quantidade <= 5) {
            ++poucos;
        }
    }
    const bool valida = estoqueApp->validar();
    const std::wstring valores[4] = {
        formatarNumero(static_cast<long long>(estoqueApp->tamanho())),
        formatarNumero(unidades),
        formatarNumero(poucos),
        formatarNumero(estoqueApp->altura())
    };
    const std::wstring descricaoArvore = L"Altura negra " + formatarNumero(estoqueApp->alturaNegra()) +
                                         (valida ? L" | rubro-negra válida" : L" | INVÁLIDA");

    const int espaco = 16;
    const int larguraCartao = (area.right - MARGEM * 2 - espaco * 3) / 4;
    RECT cartoes[4];
    for (int i = 0; i < 4; ++i) {
        const int esquerda = MARGEM + i * (larguraCartao + espaco);
        cartoes[i] = RECT{esquerda, TOPO_CARTOES, esquerda + larguraCartao, BASE_CARTOES};
    }
    desenharCartao(dc, cartoes[0], L"CATÁLOGO", valores[0].c_str(), L"Produtos cadastrados");
    desenharCartao(dc, cartoes[1], L"UNIDADES EM ESTOQUE", valores[1].c_str(), L"Disponíveis para venda");
    desenharCartao(dc, cartoes[2], L"ESTOQUE BAIXO", valores[2].c_str(), L"Produtos com 5 unidades ou menos");
    desenharCartao(dc, cartoes[3], L"ALTURA DA ÁRVORE", valores[3].c_str(), descricaoArvore.c_str());

    desenharTexto(dc, L"Produtos", 35, TOPO_TITULO - 4, COR_TEXTO, 14, FW_BOLD);
    const RECT busca = retanguloBusca(area.right);
    desenharTexto(dc, L"Buscar:", busca.left - 62, busca.top + 6, COR_SUAVE, 10, FW_SEMIBOLD);
    EndPaint(janela, &ps);
}

bool criarControles(HWND janela) {
    fonteApp = CreateFontW(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                           DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    // As posições definitivas são calculadas em ajustarLayout (WM_SIZE).
    for (const Botao& botao : BOTOES) {
        criarControle(BS_OWNERDRAW | WS_TABSTOP, L"BUTTON", botao.texto, botao.id, 0, 0, 0, 0, janela);
    }
    buscaApp = criarControle(WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL, L"EDIT", L"", ID_BUSCA, 0, 0, 0, 0, janela);
    tabelaApp = CreateWindowExW(WS_EX_CLIENTEDGE, L"SysListView32", L"",
                                WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                                0, 0, 0, 0, janela,
                                reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_TABELA)),
                                GetModuleHandleW(nullptr), nullptr);
    if (tabelaApp == nullptr) {
        return false;
    }
    ListView_SetExtendedListViewStyle(tabelaApp, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    const wchar_t* cabecalhos[] = {L"CODIGO", L"PRODUTO", L"CATEGORIA", L"QUANTIDADE", L"PRECO"};
    const int larguras[] = {140, 320, 260, 150, 150};
    for (int i = 0; i < 5; ++i) {
        LVCOLUMNW coluna = {};
        coluna.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
        coluna.pszText = const_cast<LPWSTR>(cabecalhos[i]);
        coluna.cx = larguras[i];
        coluna.fmt = i >= 3 ? LVCFMT_RIGHT : LVCFMT_LEFT;
        ListView_InsertColumn(tabelaApp, i, &coluna);
    }
    criarControle(0, L"STATIC", L"Ordenar:", ID_ROTULO_ORDENAR, 0, 0, 0, 0, janela);
    ordenarApp = criarControle(CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                               L"COMBOBOX", L"", ID_ORDENAR, 0, 0, 0, 0, janela);
    SendMessageW(ordenarApp, CB_SETDROPPEDWIDTH, 180, 0);
    const wchar_t* opcoesOrdenacao[] = {
        L"Código (padrão)", L"Menor estoque", L"Maior estoque", L"Nome (A-Z)", L"Nome (Z-A)"
    };
    for (const wchar_t* opcao : opcoesOrdenacao) {
        SendMessageW(ordenarApp, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(opcao));
    }
    SendMessageW(ordenarApp, CB_SETMINVISIBLE, 5, 0);
    SendMessageW(ordenarApp, CB_SETCURSEL, 0, 0);

    criarControle(0, L"STATIC", L"Categoria:", ID_ROTULO_CATEGORIA, 0, 0, 0, 0, janela);
    categoriaApp = criarControle(CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                                 L"COMBOBOX", L"", ID_CATEGORIA, 0, 0, 0, 0, janela);
    SendMessageW(categoriaApp, CB_SETDROPPEDWIDTH, 260, 0);
    SendMessageW(tabelaApp, WM_SETFONT, reinterpret_cast<WPARAM>(fonteApp), TRUE);
    SetWindowPos(ordenarApp, HWND_TOP, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    SetWindowPos(categoriaApp, HWND_TOP, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    return true;
}

LRESULT CALLBACK procedimentoPrincipal(HWND janela, UINT mensagem, WPARAM wParam, LPARAM lParam) {
    switch (mensagem) {
    case WM_CREATE:
        janelaPrincipal = janela;
        if (!criarControles(janela)) {
            MessageBoxW(janela, L"Nao foi possivel criar a tabela de produtos.", L"Erro", MB_OK | MB_ICONERROR);
            return -1;
        }
        return 0;
    case WM_SIZE:
        ajustarLayout(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_GETMINMAXINFO: {
        MINMAXINFO* limites = reinterpret_cast<MINMAXINFO*>(lParam);
        limites->ptMinTrackSize.x = 1080;
        limites->ptMinTrackSize.y = 680;
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;  // O fundo inteiro é pintado em WM_PAINT; evita cintilação.
    case WM_PAINT:
        pintarJanela(janela);
        return 0;
    case WM_DRAWITEM:
        desenharBotao(reinterpret_cast<const DRAWITEMSTRUCT*>(lParam));
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BUSCA:
            if (HIWORD(wParam) == EN_CHANGE) {
                atualizarTabela();
            }
            break;
        case ID_NOVO: adicionarProduto(); break;
        case ID_EDITAR: editarProduto(); break;
        case ID_ATUALIZAR: atualizarQuantidade(); break;
        case ID_REMOVER: removerProduto(); break;
        case ID_IMPORTAR: importarArquivo(); break;
        case ID_EXPORTAR: exportarArquivo(); break;
        case ID_ARVORE: mostrarJanelaArvore(janelaPrincipal, *estoqueApp); break;
        case ID_ORDENAR:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                atualizarTabela();
            }
            break;
        case ID_CATEGORIA:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                atualizarTabela();
            }
            break;
        default: break;
        }
        return 0;
    case WM_NOTIFY: {
        const NMHDR* notificacao = reinterpret_cast<const NMHDR*>(lParam);
        if (notificacao->idFrom == ID_TABELA && notificacao->code == static_cast<UINT>(NM_DBLCLK)) {
            editarProduto();
            return 0;
        }
        break;
    }
    case WM_DESTROY:
        if (fonteApp != nullptr) {
            DeleteObject(fonteApp);
            fonteApp = nullptr;
        }
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcW(janela, mensagem, wParam, lParam);
}

}  // namespace

int WINAPI WinMain(HINSTANCE instancia, HINSTANCE, LPSTR, int mostrar) {
    INITCOMMONCONTROLSEX controles = {sizeof(controles), ICC_LISTVIEW_CLASSES};
    if (!InitCommonControlsEx(&controles)) {
        MessageBoxW(nullptr, L"Nao foi possivel inicializar os controles da interface.",
                    L"Erro", MB_OK | MB_ICONERROR);
        return 1;
    }

    ArvoreRubroNegra estoque;
    estoqueApp = &estoque;
    std::ostringstream avisosCarga;
    const bool carregou = carregarEstoque(estoque, avisosCarga);

    WNDCLASSW classe = {};
    classe.lpfnWndProc = procedimentoPrincipal;
    classe.hInstance = instancia;
    classe.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    classe.hbrBackground = nullptr;
    classe.lpszClassName = L"EstoqueJanelaPrincipal";
    classe.hIcon = LoadIconW(nullptr, MAKEINTRESOURCEW(32512));
    if (!RegisterClassW(&classe) || !registrarClasseFormulario(instancia) || !registrarClasseArvore(instancia)) {
        MessageBoxW(nullptr, L"Nao foi possivel iniciar a interface grafica.", L"Erro", MB_OK | MB_ICONERROR);
        return 1;
    }
    HWND janela = CreateWindowExW(0, classe.lpszClassName, L"Estoque | Gestao inteligente",
                                  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
                                  1180, 800, nullptr, nullptr, instancia, nullptr);
    if (janela == nullptr) {
        MessageBoxW(nullptr, L"Nao foi possivel criar a janela principal.", L"Erro", MB_OK | MB_ICONERROR);
        return 1;
    }
    ShowWindow(janela, mostrar);
    UpdateWindow(janela);
    atualizarTabela();

    const std::string avisos = avisosCarga.str();
    if (!avisos.empty()) {
        const std::wstring titulo = carregou ? L"Avisos ao carregar o estoque" : L"Falha ao carregar o estoque";
        MessageBoxW(janela, paraWide(avisos).c_str(), titulo.c_str(),
                    MB_OK | (carregou ? MB_ICONWARNING : MB_ICONERROR));
    }

    MSG mensagem;
    int resultado;
    while ((resultado = GetMessageW(&mensagem, nullptr, 0, 0)) > 0) {
        TranslateMessage(&mensagem);
        DispatchMessageW(&mensagem);
    }
    if (resultado == -1) {
        MessageBoxW(nullptr, L"O loop da janela foi interrompido por um erro.", L"Erro", MB_OK | MB_ICONERROR);
        return 1;
    }
    return static_cast<int>(mensagem.wParam);
}
