#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#ifdef ESTOQUE_GUI
#define _WIN32_IE 0x0500
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#endif

struct Produto {
    std::string codigo;
    std::string nome;
    std::string categoria;
    int quantidade;
    double preco;
};

class ArvoreRubroNegra {
private:
    enum Cor { VERMELHO, PRETO };

    struct No {
        Produto produto;
        Cor cor;
        No* esquerda;
        No* direita;
        No* pai;

        explicit No(const Produto& produto, Cor cor = VERMELHO)
            : produto(produto), cor(cor), esquerda(nullptr), direita(nullptr), pai(nullptr) {}
    };

    No* sentinela;
    No* raiz;

    No* buscarNo(const std::string& codigo) const {
        No* atual = raiz;
        while (atual != sentinela && codigo != atual->produto.codigo) {
            atual = codigo < atual->produto.codigo ? atual->esquerda : atual->direita;
        }
        return atual;
    }

    void rotacionarEsquerda(No* no) {
        No* filho = no->direita;
        no->direita = filho->esquerda;
        if (filho->esquerda != sentinela) {
            filho->esquerda->pai = no;
        }
        filho->pai = no->pai;
        if (no->pai == sentinela) {
            raiz = filho;
        } else if (no == no->pai->esquerda) {
            no->pai->esquerda = filho;
        } else {
            no->pai->direita = filho;
        }
        filho->esquerda = no;
        no->pai = filho;
    }

    void rotacionarDireita(No* no) {
        No* filho = no->esquerda;
        no->esquerda = filho->direita;
        if (filho->direita != sentinela) {
            filho->direita->pai = no;
        }
        filho->pai = no->pai;
        if (no->pai == sentinela) {
            raiz = filho;
        } else if (no == no->pai->direita) {
            no->pai->direita = filho;
        } else {
            no->pai->esquerda = filho;
        }
        filho->direita = no;
        no->pai = filho;
    }

    void corrigirInsercao(No* no) {
        while (no->pai->cor == VERMELHO) {
            if (no->pai == no->pai->pai->esquerda) {
                No* tio = no->pai->pai->direita;
                if (tio->cor == VERMELHO) {
                    no->pai->cor = PRETO;
                    tio->cor = PRETO;
                    no->pai->pai->cor = VERMELHO;
                    no = no->pai->pai;
                } else {
                    if (no == no->pai->direita) {
                        no = no->pai;
                        rotacionarEsquerda(no);
                    }
                    no->pai->cor = PRETO;
                    no->pai->pai->cor = VERMELHO;
                    rotacionarDireita(no->pai->pai);
                }
            } else {
                No* tio = no->pai->pai->esquerda;
                if (tio->cor == VERMELHO) {
                    no->pai->cor = PRETO;
                    tio->cor = PRETO;
                    no->pai->pai->cor = VERMELHO;
                    no = no->pai->pai;
                } else {
                    if (no == no->pai->esquerda) {
                        no = no->pai;
                        rotacionarDireita(no);
                    }
                    no->pai->cor = PRETO;
                    no->pai->pai->cor = VERMELHO;
                    rotacionarEsquerda(no->pai->pai);
                }
            }
        }
        raiz->cor = PRETO;
    }

    void substituir(No* antigo, No* novo) {
        if (antigo->pai == sentinela) {
            raiz = novo;
        } else if (antigo == antigo->pai->esquerda) {
            antigo->pai->esquerda = novo;
        } else {
            antigo->pai->direita = novo;
        }
        novo->pai = antigo->pai;
    }

    No* menor(No* no) const {
        while (no->esquerda != sentinela) {
            no = no->esquerda;
        }
        return no;
    }

    void corrigirRemocao(No* no) {
        while (no != raiz && no->cor == PRETO) {
            if (no == no->pai->esquerda) {
                No* irmao = no->pai->direita;
                if (irmao->cor == VERMELHO) {
                    irmao->cor = PRETO;
                    no->pai->cor = VERMELHO;
                    rotacionarEsquerda(no->pai);
                    irmao = no->pai->direita;
                }
                if (irmao->esquerda->cor == PRETO && irmao->direita->cor == PRETO) {
                    irmao->cor = VERMELHO;
                    no = no->pai;
                } else {
                    if (irmao->direita->cor == PRETO) {
                        irmao->esquerda->cor = PRETO;
                        irmao->cor = VERMELHO;
                        rotacionarDireita(irmao);
                        irmao = no->pai->direita;
                    }
                    irmao->cor = no->pai->cor;
                    no->pai->cor = PRETO;
                    irmao->direita->cor = PRETO;
                    rotacionarEsquerda(no->pai);
                    no = raiz;
                }
            } else {
                No* irmao = no->pai->esquerda;
                if (irmao->cor == VERMELHO) {
                    irmao->cor = PRETO;
                    no->pai->cor = VERMELHO;
                    rotacionarDireita(no->pai);
                    irmao = no->pai->esquerda;
                }
                if (irmao->direita->cor == PRETO && irmao->esquerda->cor == PRETO) {
                    irmao->cor = VERMELHO;
                    no = no->pai;
                } else {
                    if (irmao->esquerda->cor == PRETO) {
                        irmao->direita->cor = PRETO;
                        irmao->cor = VERMELHO;
                        rotacionarEsquerda(irmao);
                        irmao = no->pai->esquerda;
                    }
                    irmao->cor = no->pai->cor;
                    no->pai->cor = PRETO;
                    irmao->esquerda->cor = PRETO;
                    rotacionarDireita(no->pai);
                    no = raiz;
                }
            }
        }
        no->cor = PRETO;
    }

    void listarEmOrdem(No* no) const {
        if (no == sentinela) {
            return;
        }
        listarEmOrdem(no->esquerda);
        imprimirProduto(no->produto);
        listarEmOrdem(no->direita);
    }

    void coletarEmOrdem(No* no, std::vector<Produto>& produtos) const {
        if (no == sentinela) {
            return;
        }
        coletarEmOrdem(no->esquerda, produtos);
        produtos.push_back(no->produto);
        coletarEmOrdem(no->direita, produtos);
    }

    void liberarNos(No* no) {
        if (no == sentinela) {
            return;
        }
        liberarNos(no->esquerda);
        liberarNos(no->direita);
        delete no;
    }

    static void imprimirProduto(const Produto& produto) {
        std::cout << std::left << std::setw(16) << produto.codigo
                  << std::setw(28) << produto.nome
                  << std::setw(20) << produto.categoria
                  << std::right << std::setw(10) << produto.quantidade
                  << std::setw(12) << std::fixed << std::setprecision(2) << produto.preco
                  << '\n';
    }

public:
    ArvoreRubroNegra() {
        sentinela = new No(Produto{"", "", "", 0, 0.0}, PRETO);
        sentinela->esquerda = sentinela;
        sentinela->direita = sentinela;
        sentinela->pai = sentinela;
        raiz = sentinela;
    }

    ~ArvoreRubroNegra() {
        liberarNos(raiz);
        delete sentinela;
    }

    ArvoreRubroNegra(const ArvoreRubroNegra&) = delete;
    ArvoreRubroNegra& operator=(const ArvoreRubroNegra&) = delete;

    bool inserir(const Produto& produto) {
        No* pai = sentinela;
        No* atual = raiz;
        while (atual != sentinela) {
            pai = atual;
            if (produto.codigo == atual->produto.codigo) {
                return false;
            }
            atual = produto.codigo < atual->produto.codigo ? atual->esquerda : atual->direita;
        }

        No* novo = new No(produto);
        novo->esquerda = sentinela;
        novo->direita = sentinela;
        novo->pai = pai;
        if (pai == sentinela) {
            raiz = novo;
        } else if (produto.codigo < pai->produto.codigo) {
            pai->esquerda = novo;
        } else {
            pai->direita = novo;
        }
        corrigirInsercao(novo);
        return true;
    }

    const Produto* buscar(const std::string& codigo) const {
        No* no = buscarNo(codigo);
        return no == sentinela ? nullptr : &no->produto;
    }

    std::vector<Produto> produtosEmOrdem() const {
        std::vector<Produto> produtos;
        coletarEmOrdem(raiz, produtos);
        return produtos;
    }

    bool atualizarQuantidade(const std::string& codigo, int quantidade) {
        No* no = buscarNo(codigo);
        if (no == sentinela) {
            return false;
        }
        no->produto.quantidade = quantidade;
        return true;
    }

    bool remover(const std::string& codigo) {
        No* alvo = buscarNo(codigo);
        if (alvo == sentinela) {
            return false;
        }

        No* removido = alvo;
        Cor corOriginal = removido->cor;
        No* filho;
        if (alvo->esquerda == sentinela) {
            filho = alvo->direita;
            substituir(alvo, alvo->direita);
        } else if (alvo->direita == sentinela) {
            filho = alvo->esquerda;
            substituir(alvo, alvo->esquerda);
        } else {
            removido = menor(alvo->direita);
            corOriginal = removido->cor;
            filho = removido->direita;
            if (removido->pai == alvo) {
                filho->pai = removido;
            } else {
                substituir(removido, removido->direita);
                removido->direita = alvo->direita;
                removido->direita->pai = removido;
            }
            substituir(alvo, removido);
            removido->esquerda = alvo->esquerda;
            removido->esquerda->pai = removido;
            removido->cor = alvo->cor;
        }
        delete alvo;
        if (corOriginal == PRETO) {
            corrigirRemocao(filho);
        }
        sentinela->pai = sentinela;
        return true;
    }

    void listar() const {
        if (raiz == sentinela) {
            std::cout << "Estoque vazio.\n";
            return;
        }
        std::cout << std::left << std::setw(16) << "Codigo"
                  << std::setw(28) << "Nome"
                  << std::setw(20) << "Categoria"
                  << std::right << std::setw(10) << "Quantidade"
                  << std::setw(12) << "Preco\n";
        listarEmOrdem(raiz);
    }
};

static std::string aparar(const std::string& texto) {
    const std::string espacos = " \t\r\n";
    const std::size_t inicio = texto.find_first_not_of(espacos);
    if (inicio == std::string::npos) {
        return "";
    }
    return texto.substr(inicio, texto.find_last_not_of(espacos) - inicio + 1);
}

static bool separarCsv(const std::string& linha, std::vector<std::string>& campos) {
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

static bool converterInteiro(const std::string& texto, int& valor) {
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

static bool converterPreco(const std::string& texto, double& valor) {
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

static bool importarCsv(const std::string& caminho, ArvoreRubroNegra& estoque,
                        std::ostream& saida = std::cout, std::ostream& erros = std::cerr) {
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
    std::size_t inseridos = 0;
    std::size_t ignorados = 0;
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
            ++ignorados;
            continue;
        }
        if (!estoque.inserir(Produto{campos[0], campos[1], campos[2], quantidade, preco})) {
            erros << "Aviso: codigo duplicado na linha " << linhaAtual << " ("
                  << campos[0] << "); ignorado.\n";
            ++ignorados;
            continue;
        }
        ++inseridos;
    }
    if (arquivo.bad()) {
        erros << "Erro de leitura durante a importacao do arquivo.\n";
        return false;
    }
    saida << "Importacao concluida: " << inseridos << " produto(s) inserido(s), "
          << ignorados << " linha(s) ignorada(s).\n";
    return true;
}

#ifndef ESTOQUE_GUI
static std::string lerTexto(const std::string& mensagem) {
    std::cout << mensagem;
    std::string texto;
    std::getline(std::cin, texto);
    return aparar(texto);
}

static bool lerQuantidade(const std::string& mensagem, int& quantidade) {
    const std::string texto = lerTexto(mensagem);
    return converterInteiro(texto, quantidade) && quantidade >= 0;
}

static bool lerProduto(Produto& produto) {
    produto.codigo = lerTexto("Codigo: ");
    produto.nome = lerTexto("Nome: ");
    produto.categoria = lerTexto("Categoria: ");
    if (produto.codigo.empty() || produto.nome.empty() || produto.categoria.empty()) {
        std::cout << "Codigo, nome e categoria sao obrigatorios.\n";
        return false;
    }
    if (!lerQuantidade("Quantidade: ", produto.quantidade)) {
        std::cout << "Quantidade invalida. Informe um inteiro nao negativo.\n";
        return false;
    }
    if (!converterPreco(lerTexto("Preco: "), produto.preco)) {
        std::cout << "Preco invalido. Informe um numero nao negativo usando ponto decimal.\n";
        return false;
    }
    return true;
}

static void exibirMenu() {
    std::cout << "\n=== Gerenciador de Estoque (arvore rubro-negra) ===\n"
              << "1. Cadastrar produto\n"
              << "2. Consultar produto\n"
              << "3. Listar estoque\n"
              << "4. Atualizar quantidade\n"
              << "5. Remover produto\n"
              << "6. Importar CSV\n"
              << "0. Sair\n";
}

int main() {
    ArvoreRubroNegra estoque;
    std::string opcao;
    do {
        exibirMenu();
        opcao = lerTexto("Opcao: ");
        if (opcao == "1") {
            Produto produto;
            if (lerProduto(produto)) {
                if (estoque.inserir(produto)) {
                    std::cout << "Produto cadastrado.\n";
                } else {
                    std::cout << "Ja existe um produto com esse codigo.\n";
                }
            }
        } else if (opcao == "2") {
            const Produto* produto = estoque.buscar(lerTexto("Codigo: "));
            if (produto == nullptr) {
                std::cout << "Produto nao encontrado.\n";
            } else {
                std::cout << std::left << std::setw(16) << "Codigo"
                          << std::setw(28) << "Nome"
                          << std::setw(20) << "Categoria"
                          << std::right << std::setw(10) << "Quantidade"
                          << std::setw(12) << "Preco\n";
                std::cout << std::left << std::setw(16) << produto->codigo
                          << std::setw(28) << produto->nome
                          << std::setw(20) << produto->categoria
                          << std::right << std::setw(10) << produto->quantidade
                          << std::setw(12) << std::fixed << std::setprecision(2)
                          << produto->preco << '\n';
            }
        } else if (opcao == "3") {
            estoque.listar();
        } else if (opcao == "4") {
            const std::string codigo = lerTexto("Codigo: ");
            int quantidade = 0;
            if (!lerQuantidade("Nova quantidade: ", quantidade)) {
                std::cout << "Quantidade invalida. Informe um inteiro nao negativo.\n";
            } else if (!estoque.atualizarQuantidade(codigo, quantidade)) {
                std::cout << "Produto nao encontrado.\n";
            } else {
                std::cout << "Quantidade atualizada.\n";
            }
        } else if (opcao == "5") {
            if (estoque.remover(lerTexto("Codigo: "))) {
                std::cout << "Produto removido.\n";
            } else {
                std::cout << "Produto nao encontrado.\n";
            }
        } else if (opcao == "6") {
            importarCsv(lerTexto("Caminho do arquivo CSV: "), estoque);
        } else if (opcao != "0") {
            std::cout << "Opcao invalida.\n";
        }
    } while (opcao != "0" && !std::cin.eof());

    std::cout << "Programa encerrado.\n";
    return 0;
}
#else

enum Controle {
    ID_BUSCA = 100,
    ID_TABELA,
    ID_NOVO,
    ID_ATUALIZAR,
    ID_REMOVER,
    ID_IMPORTAR,
    ID_FORM_CODIGO = 200,
    ID_FORM_NOME,
    ID_FORM_CATEGORIA,
    ID_FORM_QUANTIDADE,
    ID_FORM_PRECO,
    ID_FORM_SALVAR,
    ID_FORM_CANCELAR
};

struct DadosFormulario {
    bool aceito;
    bool apenasQuantidade;
    std::wstring titulo;
    Produto produto;
};

static ArvoreRubroNegra* estoqueApp = nullptr;
static HWND janelaPrincipal = nullptr;
static HWND buscaApp = nullptr;
static HWND tabelaApp = nullptr;
static HFONT fonteApp = nullptr;
static DadosFormulario formulario;
static HWND janelaFormulario = nullptr;
static HWND camposFormulario[5] = {};
static const COLORREF fundoApp = RGB(245, 248, 246);
static const COLORREF verdeApp = RGB(20, 67, 55);
static const COLORREF textoApp = RGB(34, 49, 43);
static const COLORREF suaveApp = RGB(111, 128, 119);
static const COLORREF destaqueApp = RGB(37, 132, 99);

static std::wstring paraWide(const std::string& texto) {
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

static std::string paraUtf8(const std::wstring& texto) {
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

static std::wstring formatarNumero(long long numero) {
    std::wostringstream fluxo;
    fluxo << numero;
    return fluxo.str();
}

static std::wstring formatarPreco(double preco) {
    std::wostringstream fluxo;
    fluxo << std::fixed << std::setprecision(2) << preco;
    return fluxo.str();
}

static void definirFonte(HWND controle) {
    SendMessageW(controle, WM_SETFONT, reinterpret_cast<WPARAM>(fonteApp), TRUE);
}

static HWND criarControle(DWORD estilo, const wchar_t* classe, const wchar_t* texto,
                          int id, int x, int y, int largura, int altura, HWND pai) {
    HWND controle = CreateWindowExW(0, classe, texto, WS_CHILD | WS_VISIBLE | estilo,
                                    x, y, largura, altura, pai,
                                    reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                                    GetModuleHandleW(nullptr), nullptr);
    definirFonte(controle);
    return controle;
}

static void desenharTexto(HDC dc, const wchar_t* texto, int x, int y, COLORREF cor,
                          int tamanho = 10, int peso = FW_NORMAL) {
    HFONT fonte = CreateFontW(-MulDiv(tamanho, GetDeviceCaps(dc, LOGPIXELSY), 72),
                              0, 0, 0, peso, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                              OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                              DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HFONT anterior = static_cast<HFONT>(SelectObject(dc, fonte));
    SetTextColor(dc, cor);
    SetBkMode(dc, TRANSPARENT);
    TextOutW(dc, x, y, texto, static_cast<int>(wcslen(texto)));
    SelectObject(dc, anterior);
    DeleteObject(fonte);
}

static void desenharCartao(HDC dc, RECT retangulo, const wchar_t* rotulo,
                           const wchar_t* valor, const wchar_t* descricao) {
    HBRUSH branco = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(dc, &retangulo, branco);
    DeleteObject(branco);
    desenharTexto(dc, rotulo, retangulo.left + 18, retangulo.top + 13, suaveApp, 9, FW_SEMIBOLD);
    desenharTexto(dc, valor, retangulo.left + 18, retangulo.top + 33, textoApp, 21, FW_BOLD);
    desenharTexto(dc, descricao, retangulo.left + 18, retangulo.top + 67, suaveApp, 9);
}

static void ajustarLayout(int largura, int altura) {
    if (buscaApp == nullptr) {
        return;
    }
    const int margem = 34;
    const int botaoY = 322;
    const int larguraBotao = 156;
    MoveWindow(GetDlgItem(janelaPrincipal, ID_NOVO), margem, botaoY, larguraBotao, 40, TRUE);
    MoveWindow(GetDlgItem(janelaPrincipal, ID_ATUALIZAR), margem + 168, botaoY, 170, 40, TRUE);
    MoveWindow(GetDlgItem(janelaPrincipal, ID_REMOVER), margem + 350, botaoY, 145, 40, TRUE);
    MoveWindow(GetDlgItem(janelaPrincipal, ID_IMPORTAR), margem + 507, botaoY, 155, 40, TRUE);
    const int buscaX = margem + 682;
    MoveWindow(buscaApp, buscaX, botaoY + 2, (std::max)(220, largura - buscaX - margem), 36, TRUE);
    MoveWindow(tabelaApp, margem, 383, (std::max)(500, largura - margem * 2),
               (std::max)(160, altura - 415), TRUE);
}

static std::wstring lerCampo(HWND controle) {
    const int tamanho = GetWindowTextLengthW(controle);
    std::wstring texto(static_cast<std::size_t>(tamanho + 1), L'\0');
    GetWindowTextW(controle, &texto[0], tamanho + 1);
    texto.resize(static_cast<std::size_t>(tamanho));
    return texto;
}

static void fecharFormulario(bool aceito) {
    if (aceito) {
        if (formulario.apenasQuantidade) {
            formulario.produto.quantidade = std::atoi(paraUtf8(lerCampo(camposFormulario[3])).c_str());
        } else {
            formulario.produto.codigo = paraUtf8(lerCampo(camposFormulario[0]));
            formulario.produto.nome = paraUtf8(lerCampo(camposFormulario[1]));
            formulario.produto.categoria = paraUtf8(lerCampo(camposFormulario[2]));
            formulario.produto.quantidade = std::atoi(paraUtf8(lerCampo(camposFormulario[3])).c_str());
            formulario.produto.preco = std::atof(paraUtf8(lerCampo(camposFormulario[4])).c_str());
        }
        formulario.aceito = true;
    }
    DestroyWindow(janelaFormulario);
}

static LRESULT CALLBACK procedimentoFormulario(HWND janela, UINT mensagem,
                                               WPARAM wParam, LPARAM lParam) {
    if (mensagem == WM_CREATE) {
        const bool quantidade = formulario.apenasQuantidade;
        if (quantidade) {
            criarControle(0, L"STATIC", L"Informe a nova quantidade em estoque:",
                          0, 28, 29, 360, 24, janela);
            camposFormulario[3] = criarControle(WS_BORDER | ES_AUTOHSCROLL, L"EDIT", L"",
                                                ID_FORM_QUANTIDADE, 28, 62, 364, 36, janela);
            SetWindowTextW(camposFormulario[3], formatarNumero(formulario.produto.quantidade).c_str());
        } else {
            const wchar_t* rotulos[] = {L"Codigo do produto", L"Nome", L"Categoria", L"Quantidade", L"Preco (R$)"};
            const std::wstring valores[] = {
                paraWide(formulario.produto.codigo),
                paraWide(formulario.produto.nome),
                paraWide(formulario.produto.categoria),
                formatarNumero(formulario.produto.quantidade),
                formatarPreco(formulario.produto.preco)
            };
            const int ids[] = {ID_FORM_CODIGO, ID_FORM_NOME, ID_FORM_CATEGORIA,
                               ID_FORM_QUANTIDADE, ID_FORM_PRECO};
            for (int i = 0; i < 5; ++i) {
                const int y = 22 + i * 58;
                criarControle(0, L"STATIC", rotulos[i], 0, 28, y, 360, 20, janela);
                camposFormulario[i] = criarControle(WS_BORDER | ES_AUTOHSCROLL, L"EDIT", valores[i].c_str(),
                                                    ids[i], 28, y + 22, 364, 32, janela);
            }
        }
        const int botoesY = quantidade ? 116 : 341;
        criarControle(BS_OWNERDRAW, L"BUTTON", L"Salvar", ID_FORM_SALVAR, 178, botoesY, 102, 38, janela);
        criarControle(BS_OWNERDRAW, L"BUTTON", L"Cancelar", ID_FORM_CANCELAR, 290, botoesY, 102, 38, janela);
        return 0;
    }
    if (mensagem == WM_COMMAND) {
        const int id = LOWORD(wParam);
        if (id == ID_FORM_CANCELAR) {
            fecharFormulario(false);
            return 0;
        }
        if (id == ID_FORM_SALVAR) {
            const std::string quantidade = paraUtf8(lerCampo(camposFormulario[3]));
            int valorQuantidade = 0;
            if (!converterInteiro(quantidade, valorQuantidade) || valorQuantidade < 0) {
                MessageBoxW(janela, L"Informe uma quantidade inteira igual ou maior que zero.",
                            L"Valor invalido", MB_OK | MB_ICONWARNING);
                SetFocus(camposFormulario[3]);
                return 0;
            }
            if (!formulario.apenasQuantidade) {
                double preco = 0.0;
                if (aparar(paraUtf8(lerCampo(camposFormulario[0]))).empty() ||
                    aparar(paraUtf8(lerCampo(camposFormulario[1]))).empty() ||
                    aparar(paraUtf8(lerCampo(camposFormulario[2]))).empty()) {
                    MessageBoxW(janela, L"Preencha o codigo, o nome e a categoria do produto.",
                                L"Campos obrigatorios", MB_OK | MB_ICONWARNING);
                    return 0;
                }
                const std::string precoTexto = paraUtf8(lerCampo(camposFormulario[4]));
                if (!converterPreco(precoTexto, preco)) {
                    MessageBoxW(janela, L"Informe um preco valido, usando ponto decimal.",
                                L"Valor invalido", MB_OK | MB_ICONWARNING);
                    SetFocus(camposFormulario[4]);
                    return 0;
                }
            }
            fecharFormulario(true);
            return 0;
        }
    }
    if (mensagem == WM_DRAWITEM) {
        DRAWITEMSTRUCT* item = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
        const bool primario = item->CtlID == ID_FORM_SALVAR;
        HBRUSH pincel = CreateSolidBrush(primario ? destaqueApp : RGB(239, 244, 241));
        FillRect(item->hDC, &item->rcItem, pincel);
        DeleteObject(pincel);
        SetBkMode(item->hDC, TRANSPARENT);
        SetTextColor(item->hDC, primario ? RGB(255, 255, 255) : textoApp);
        wchar_t texto[32];
        GetWindowTextW(item->hwndItem, texto, 32);
        DrawTextW(item->hDC, texto, -1, &item->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        return TRUE;
    }
    if (mensagem == WM_CLOSE) {
        fecharFormulario(false);
        return 0;
    }
    return DefWindowProcW(janela, mensagem, wParam, lParam);
}

static bool abrirFormulario(HWND pai, const wchar_t* titulo, const Produto& produto,
                            bool apenasQuantidade) {
    formulario.aceito = false;
    formulario.apenasQuantidade = apenasQuantidade;
    formulario.produto = produto;
    formulario.titulo = titulo;
    janelaFormulario = CreateWindowExW(WS_EX_DLGMODALFRAME, L"EstoqueFormulario",
                                       titulo, WS_CAPTION | WS_SYSMENU,
                                       CW_USEDEFAULT, CW_USEDEFAULT, 440,
                                       apenasQuantidade ? 200 : 430,
                                       pai, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (janelaFormulario == nullptr) {
        MessageBoxW(pai, L"Nao foi possivel abrir o formulario.", L"Erro", MB_OK | MB_ICONERROR);
        return false;
    }
    EnableWindow(pai, FALSE);
    ShowWindow(janelaFormulario, SW_SHOW);
    SetForegroundWindow(janelaFormulario);
    if (apenasQuantidade) {
        SetFocus(camposFormulario[3]);
    } else {
        SetFocus(camposFormulario[0]);
    }
    MSG mensagem;
    int resultado;
    while (IsWindow(janelaFormulario) &&
           (resultado = GetMessageW(&mensagem, nullptr, 0, 0)) > 0) {
        if (!IsDialogMessageW(janelaFormulario, &mensagem)) {
            TranslateMessage(&mensagem);
            DispatchMessageW(&mensagem);
        }
    }
    if (IsWindow(janelaFormulario)) {
        DestroyWindow(janelaFormulario);
        MessageBoxW(pai, L"O loop da janela foi interrompido por um erro.",
                    L"Erro", MB_OK | MB_ICONERROR);
    }
    EnableWindow(pai, TRUE);
    SetForegroundWindow(pai);
    return formulario.aceito;
}

static std::wstring filtrarTexto;

static void atualizarTabela() {
    if (tabelaApp == nullptr) {
        return;
    }
    ListView_DeleteAllItems(tabelaApp);
    filtrarTexto = lerCampo(buscaApp);
    for (const Produto& produto : estoqueApp->produtosEmOrdem()) {
        const std::wstring campos[] = {
            paraWide(produto.codigo), paraWide(produto.nome), paraWide(produto.categoria),
            formatarNumero(produto.quantidade), L"R$ " + formatarPreco(produto.preco)
        };
        std::wstring termo = filtrarTexto;
        std::wstring textoPesquisa = campos[0] + L" " + campos[1] + L" " + campos[2];
        std::transform(termo.begin(), termo.end(), termo.begin(), towlower);
        std::transform(textoPesquisa.begin(), textoPesquisa.end(), textoPesquisa.begin(), towlower);
        if (!termo.empty() && textoPesquisa.find(termo) == std::wstring::npos) {
            continue;
        }
        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = ListView_GetItemCount(tabelaApp);
        item.pszText = const_cast<LPWSTR>(campos[0].c_str());
        const int indice = ListView_InsertItem(tabelaApp, &item);
        for (int coluna = 1; coluna < 5; ++coluna) {
            ListView_SetItemText(tabelaApp, indice, coluna,
                                 const_cast<LPWSTR>(campos[coluna].c_str()));
        }
    }
    InvalidateRect(janelaPrincipal, nullptr, TRUE);
}

static int indiceSelecionado() {
    return ListView_GetNextItem(tabelaApp, -1, LVNI_SELECTED);
}

static std::string codigoSelecionado() {
    const int indice = indiceSelecionado();
    if (indice < 0) {
        return std::string();
    }
    wchar_t codigo[256] = {};
    ListView_GetItemText(tabelaApp, indice, 0, codigo, 256);
    return paraUtf8(codigo);
}

static void adicionarProduto() {
    Produto novo = {"", "", "", 0, 0.0};
    if (!abrirFormulario(janelaPrincipal, L"Adicionar produto", novo, false)) {
        return;
    }
    if (!estoqueApp->inserir(formulario.produto)) {
        MessageBoxW(janelaPrincipal, L"Ja existe um produto com esse codigo.",
                    L"Codigo duplicado", MB_OK | MB_ICONWARNING);
        return;
    }
    atualizarTabela();
}

static void atualizarEstoque() {
    const std::string codigo = codigoSelecionado();
    if (codigo.empty()) {
        MessageBoxW(janelaPrincipal, L"Selecione um produto na tabela primeiro.",
                    L"Nenhum produto selecionado", MB_OK | MB_ICONINFORMATION);
        return;
    }
    const Produto* atual = estoqueApp->buscar(codigo);
    if (atual != nullptr && abrirFormulario(janelaPrincipal, L"Atualizar estoque", *atual, true)) {
        estoqueApp->atualizarQuantidade(codigo, formulario.produto.quantidade);
        atualizarTabela();
    }
}

static void removerProduto() {
    const std::string codigo = codigoSelecionado();
    if (codigo.empty()) {
        MessageBoxW(janelaPrincipal, L"Selecione um produto na tabela primeiro.",
                    L"Nenhum produto selecionado", MB_OK | MB_ICONINFORMATION);
        return;
    }
    const Produto* produto = estoqueApp->buscar(codigo);
    const std::wstring pergunta = L"Remover \"" +
        (produto == nullptr ? std::wstring() : paraWide(produto->nome)) + L"\" do estoque?";
    if (MessageBoxW(janelaPrincipal, pergunta.c_str(), L"Confirmar remocao",
                    MB_YESNO | MB_ICONQUESTION) == IDYES) {
        estoqueApp->remover(codigo);
        atualizarTabela();
    }
}

static void importarArquivo() {
    wchar_t caminho[MAX_PATH] = {};
    OPENFILENAMEW dialogo = {};
    dialogo.lStructSize = sizeof(dialogo);
    dialogo.hwndOwner = janelaPrincipal;
    dialogo.lpstrFilter = L"Arquivos CSV (*.csv)\0*.csv\0Todos os arquivos (*.*)\0*.*\0";
    dialogo.lpstrFile = caminho;
    dialogo.nMaxFile = MAX_PATH;
    dialogo.lpstrTitle = L"Importar produtos do arquivo CSV";
    dialogo.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    if (!GetOpenFileNameW(&dialogo)) {
        return;
    }
    std::ostringstream saida;
    std::ostringstream erros;
    if (!importarCsv(paraUtf8(caminho), *estoqueApp, saida, erros)) {
        const std::string mensagem = erros.str();
        MessageBoxW(janelaPrincipal, paraWide(mensagem).c_str(), L"Falha ao importar CSV",
                    MB_OK | MB_ICONERROR);
        return;
    }
    atualizarTabela();
    const std::string avisos = erros.str();
    const std::string mensagem = saida.str() +
        (avisos.empty() ? std::string() : "\n\nAvisos:\n" + avisos);
    MessageBoxW(janelaPrincipal, paraWide(mensagem).c_str(), L"Importacao concluida",
                MB_OK | (avisos.empty() ? MB_ICONINFORMATION : MB_ICONWARNING));
}

static void desenharBotao(DRAWITEMSTRUCT* item) {
    COLORREF cor = fundoApp;
    COLORREF texto = textoApp;
    if (item->CtlID == ID_NOVO) {
        cor = destaqueApp;
        texto = RGB(255, 255, 255);
    } else if (item->CtlID == ID_REMOVER) {
        cor = RGB(255, 241, 238);
        texto = RGB(173, 72, 60);
    }
    if (item->itemState & ODS_SELECTED) {
        cor = RGB(GetRValue(cor) * 9 / 10, GetGValue(cor) * 9 / 10, GetBValue(cor) * 9 / 10);
    }
    HBRUSH pincel = CreateSolidBrush(cor);
    HPEN caneta = CreatePen(PS_SOLID, 1, cor);
    HGDIOBJ canetaAnterior = SelectObject(item->hDC, caneta);
    HGDIOBJ pincelAnterior = SelectObject(item->hDC, pincel);
    RoundRect(item->hDC, item->rcItem.left, item->rcItem.top,
              item->rcItem.right, item->rcItem.bottom, 12, 12);
    SelectObject(item->hDC, pincelAnterior);
    SelectObject(item->hDC, canetaAnterior);
    SetBkMode(item->hDC, TRANSPARENT);
    SetTextColor(item->hDC, texto);
    wchar_t titulo[64] = {};
    GetWindowTextW(item->hwndItem, titulo, 64);
    DrawTextW(item->hDC, titulo, -1, &item->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(pincel);
    DeleteObject(caneta);
}

static LRESULT CALLBACK procedimentoPrincipal(HWND janela, UINT mensagem,
                                              WPARAM wParam, LPARAM lParam) {
    if (mensagem == WM_CREATE) {
        janelaPrincipal = janela;
        fonteApp = CreateFontW(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                               DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        criarControle(BS_OWNERDRAW, L"BUTTON", L"+  Novo produto", ID_NOVO, 34, 322, 156, 40, janela);
        criarControle(BS_OWNERDRAW, L"BUTTON", L"Atualizar estoque", ID_ATUALIZAR, 202, 322, 170, 40, janela);
        criarControle(BS_OWNERDRAW, L"BUTTON", L"Remover", ID_REMOVER, 384, 322, 145, 40, janela);
        criarControle(BS_OWNERDRAW, L"BUTTON", L"Importar CSV", ID_IMPORTAR, 541, 322, 155, 40, janela);
        buscaApp = criarControle(WS_BORDER | ES_AUTOHSCROLL, L"EDIT", L"",
                                 ID_BUSCA, 716, 324, 300, 36, janela);
        tabelaApp = CreateWindowExW(WS_EX_CLIENTEDGE, L"SysListView32", L"",
                                    WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                                    34, 383, 1090, 320, janela,
                                    reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_TABELA)),
                                    GetModuleHandleW(nullptr), nullptr);
        if (tabelaApp == nullptr) {
            MessageBoxW(janela, L"Nao foi possivel criar a tabela de produtos.",
                        L"Erro", MB_OK | MB_ICONERROR);
            return -1;
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
        SendMessageW(tabelaApp, WM_SETFONT, reinterpret_cast<WPARAM>(fonteApp), TRUE);
        return 0;
    }
    if (mensagem == WM_SIZE) {
        ajustarLayout(LOWORD(lParam), HIWORD(lParam));
        return 0;
    }
    if (mensagem == WM_GETMINMAXINFO) {
        MINMAXINFO* limites = reinterpret_cast<MINMAXINFO*>(lParam);
        limites->ptMinTrackSize.x = 1020;
        limites->ptMinTrackSize.y = 680;
        return 0;
    }
    if (mensagem == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(janela, &ps);
        RECT area;
        GetClientRect(janela, &area);
        HBRUSH fundo = CreateSolidBrush(fundoApp);
        FillRect(dc, &area, fundo);
        DeleteObject(fundo);
        RECT hero = {0, 0, area.right, 165};
        HBRUSH verde = CreateSolidBrush(verdeApp);
        FillRect(dc, &hero, verde);
        DeleteObject(verde);
        desenharTexto(dc, L"PAINEL DE ESTOQUE", 36, 22, RGB(181, 221, 201), 9, FW_BOLD);
        desenharTexto(dc, L"Seu estoque, sob controle.", 34, 48, RGB(255, 255, 255), 24, FW_BOLD);
        desenharTexto(dc, L"Gerencie produtos com rapidez e acompanhe o que precisa de reposicao.",
                      36, 91, RGB(218, 234, 224), 10);
        const int margem = 34;
        const int espaco = 16;
        const int cardWidth = (area.right - margem * 2 - espaco * 2) / 3;
        const std::vector<Produto> produtos = estoqueApp->produtosEmOrdem();
        long long unidades = 0;
        int poucos = 0;
        for (const Produto& produto : produtos) {
            unidades += produto.quantidade;
            if (produto.quantidade <= 5) {
                ++poucos;
            }
        }
        wchar_t valores[3][64];
        wcscpy(valores[0], formatarNumero(static_cast<long long>(produtos.size())).c_str());
        wcscpy(valores[1], formatarNumero(unidades).c_str());
        wcscpy(valores[2], formatarNumero(poucos).c_str());
        RECT cartoes[3];
        for (int i = 0; i < 3; ++i) {
            cartoes[i] = RECT{margem + i * (cardWidth + espaco), 190,
                              margem + i * (cardWidth + espaco) + cardWidth, 288};
        }
        desenharCartao(dc, cartoes[0], L"CATALOGO", valores[0], L"Produtos cadastrados no estoque");
        desenharCartao(dc, cartoes[1], L"UNIDADES EM ESTOQUE", valores[1], L"Unidades disponiveis para venda");
        desenharCartao(dc, cartoes[2], L"ESTOQUE BAIXO", valores[2], L"Produtos com 5 unidades ou menos");
        desenharTexto(dc, L"Produtos", 35, 296, textoApp, 14, FW_BOLD);
        EndPaint(janela, &ps);
        return 0;
    }
    if (mensagem == WM_DRAWITEM) {
        desenharBotao(reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
        return TRUE;
    }
    if (mensagem == WM_COMMAND) {
        const int id = LOWORD(wParam);
        if (id == ID_BUSCA && HIWORD(wParam) == EN_CHANGE) {
            atualizarTabela();
        } else if (id == ID_NOVO) {
            adicionarProduto();
        } else if (id == ID_ATUALIZAR) {
            atualizarEstoque();
        } else if (id == ID_REMOVER) {
            removerProduto();
        } else if (id == ID_IMPORTAR) {
            importarArquivo();
        }
        return 0;
    }
    if (mensagem == WM_NOTIFY) {
        const NMHDR* notificacao = reinterpret_cast<NMHDR*>(lParam);
        if (notificacao->idFrom == ID_TABELA &&
            notificacao->code == static_cast<UINT>(NM_DBLCLK)) {
            atualizarEstoque();
            return 0;
        }
    }
    if (mensagem == WM_DESTROY) {
        if (fonteApp != nullptr) {
            DeleteObject(fonteApp);
            fonteApp = nullptr;
        }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(janela, mensagem, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE instancia, HINSTANCE, LPSTR, int mostrar) {
    INITCOMMONCONTROLSEX controles = {sizeof(controles), ICC_LISTVIEW_CLASSES};
    if (!InitCommonControlsEx(&controles)) {
        MessageBoxW(nullptr, L"Nao foi possivel inicializar os controles da interface.",
                    L"Erro", MB_OK | MB_ICONERROR);
        return 1;
    }
    ArvoreRubroNegra estoque;
    estoqueApp = &estoque;
    WNDCLASSW classeFormulario = {};
    classeFormulario.lpfnWndProc = procedimentoFormulario;
    classeFormulario.hInstance = instancia;
    classeFormulario.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    classeFormulario.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
    classeFormulario.lpszClassName = L"EstoqueFormulario";
    RegisterClassW(&classeFormulario);

    WNDCLASSW classe = {};
    classe.lpfnWndProc = procedimentoPrincipal;
    classe.hInstance = instancia;
    classe.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    classe.hbrBackground = CreateSolidBrush(fundoApp);
    classe.lpszClassName = L"EstoqueJanelaPrincipal";
    classe.hIcon = LoadIconW(nullptr, MAKEINTRESOURCEW(32512));
    if (!RegisterClassW(&classe)) {
        MessageBoxW(nullptr, L"Nao foi possivel iniciar a interface grafica.",
                    L"Erro", MB_OK | MB_ICONERROR);
        return 1;
    }
    HWND janela = CreateWindowExW(0, classe.lpszClassName, L"Estoque | Gestao inteligente",
                                 WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                 1180, 780, nullptr, nullptr, instancia, nullptr);
    if (janela == nullptr) {
        MessageBoxW(nullptr, L"Nao foi possivel criar a janela principal.",
                    L"Erro", MB_OK | MB_ICONERROR);
        return 1;
    }
    ShowWindow(janela, mostrar);
    UpdateWindow(janela);
    atualizarTabela();
    MSG mensagem;
    int resultado;
    while ((resultado = GetMessageW(&mensagem, nullptr, 0, 0)) > 0) {
        TranslateMessage(&mensagem);
        DispatchMessageW(&mensagem);
    }
    DeleteObject(classeFormulario.hbrBackground);
    DeleteObject(classe.hbrBackground);
    if (resultado == -1) {
        MessageBoxW(nullptr, L"O loop da janela foi interrompido por um erro.",
                    L"Erro", MB_OK | MB_ICONERROR);
        return 1;
    }
    return static_cast<int>(mensagem.wParam);
}

#endif
