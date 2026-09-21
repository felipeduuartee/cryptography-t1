CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -I.
OPENSSL_LIBS := -lssl -lcrypto

BRBEPO_SRCS := encriptador.cpp descriptador.cpp

.PHONY: all clean graphs dados validar-dados validar-chave experimento

all: programa_BRBEPO benchmark gerador_tabela

programa_BRBEPO: main.cpp $(BRBEPO_SRCS) encriptador.h descriptador.h setup.h
	$(CXX) $(CXXFLAGS) -o $@ main.cpp $(BRBEPO_SRCS)

benchmark: benchmark.cpp $(BRBEPO_SRCS) openssl_crypto.cpp openssl_crypto.h setup.h
	$(CXX) $(CXXFLAGS) -o $@ benchmark.cpp $(BRBEPO_SRCS) openssl_crypto.cpp $(OPENSSL_LIBS)

gerador_tabela: gerador_tabela.cpp resultados_brasileirao_2002.txt
	$(CXX) $(CXXFLAGS) -o $@ gerador_tabela.cpp

validar-chave: gerador_tabela
	python3 scripts/verificar_gerador.py

dados:
	python3 scripts/preparar_gutenberg.py

validar-dados:
	python3 scripts/validar_gutenberg.py

experimento: benchmark
	python3 scripts/executar_experimento.py

graphs:
	python3 scripts/plot_benchmark.py resultados/benchmark.csv --diretorio resultados

clean:
	rm -f programa_BRBEPO benchmark gerador_tabela
	rm -f arquivoencriptado.txt arquivodescriptado.txt
