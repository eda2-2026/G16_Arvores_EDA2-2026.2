#ifndef GUI_COMUM_HPP
#define GUI_COMUM_HPP

// Definições e utilidades compartilhadas pelas janelas da interface Win32.
// Este cabeçalho precisa ser o primeiro include de cada arquivo da GUI.

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0500
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

#include <string>

// Paleta da interface.
extern const COLORREF COR_FUNDO;
extern const COLORREF COR_VERDE;
extern const COLORREF COR_TEXTO;
extern const COLORREF COR_SUAVE;
extern const COLORREF COR_DESTAQUE;

// Fonte padrão dos controles (criada pela janela principal).
extern HFONT fonteApp;

std::wstring paraWide(const std::string& texto);
std::string paraUtf8(const std::wstring& texto);
std::wstring formatarNumero(long long numero);
std::wstring formatarPreco(double preco);

// Cria um controle filho visível com a fonte padrão.
HWND criarControle(DWORD estilo, const wchar_t* classe, const wchar_t* texto,
                   int id, int x, int y, int largura, int altura, HWND pai);

// Lê o texto de um controle.
std::wstring lerCampo(HWND controle);

#endif
