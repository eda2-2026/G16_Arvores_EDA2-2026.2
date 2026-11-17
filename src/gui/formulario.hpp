#ifndef FORMULARIO_HPP
#define FORMULARIO_HPP

#include "gui_comum.hpp"
#include "produto.hpp"

enum class ModoFormulario {
    Novo,        // todos os campos editáveis
    Editar,      // código bloqueado; os demais campos podem ser editados
    Quantidade   // apenas a quantidade
};

// Registra a classe de janela do formulário. Chamar uma vez antes de usar.
bool registrarClasseFormulario(HINSTANCE instancia);

// Abre o formulário modal preenchido com `produto`. Se o usuário salvar,
// grava os valores validados em `produto` e retorna true.
bool abrirFormulario(HWND pai, const wchar_t* titulo, ModoFormulario modo, Produto& produto);

#endif
