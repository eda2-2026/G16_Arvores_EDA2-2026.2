#ifndef JANELA_ARVORE_HPP
#define JANELA_ARVORE_HPP

#include "gui_comum.hpp"
#include "arvore_rubro_negra.hpp"

// Registra a classe da janela de visualização da árvore.
bool registrarClasseArvore(HINSTANCE instancia);

// Abre (ou traz para frente) a janela que mostra a estrutura da árvore.
void mostrarJanelaArvore(HWND pai, const ArvoreRubroNegra& estoque);

// Redesenha o conteúdo se a janela estiver aberta. Chamar após alterar o estoque.
void atualizarJanelaArvore(const ArvoreRubroNegra& estoque);

#endif
