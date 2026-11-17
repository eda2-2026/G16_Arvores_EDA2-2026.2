# Uso (MinGW): mingw32-make [console|gui|teste|testar|clean]
# Em Linux/MSYS2: make console teste

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -O2
INCLUDES  = -Iinclude

NUCLEO  = src/arvore_rubro_negra.cpp src/csv.cpp src/persistencia.cpp src/utilitarios.cpp
CONSOLE = src/console/main_console.cpp
GUI     = src/gui/main_gui.cpp src/gui/gui_comum.cpp src/gui/formulario.cpp src/gui/janela_arvore.cpp
TESTES  = tests/teste_arvore.cpp
CABECALHOS = $(wildcard include/*.hpp) $(wildcard src/gui/*.hpp)

.PHONY: all console gui teste testar clean

all: console gui teste

console: estoque_console.exe
gui: estoque.exe
teste: teste_arvore.exe

estoque_console.exe: $(NUCLEO) $(CONSOLE) $(CABECALHOS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(NUCLEO) $(CONSOLE) -o $@

estoque.exe: $(NUCLEO) $(GUI) $(CABECALHOS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(NUCLEO) $(GUI) -mwindows -lcomctl32 -lcomdlg32 -o $@

teste_arvore.exe: $(NUCLEO) $(TESTES) $(CABECALHOS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(NUCLEO) $(TESTES) -o $@

testar: teste_arvore.exe
	./teste_arvore.exe

clean:
	-$(RM) estoque_console.exe estoque.exe teste_arvore.exe
