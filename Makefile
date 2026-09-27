CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -I.
OPENSSL_LIBS := -lssl -lcrypto

BRBEPO_SRCS := encriptador.cpp descriptador.cpp

.PHONY: all clean

all: programa_BRBEPO benchmark

programa_BRBEPO: main.cpp $(BRBEPO_SRCS) encriptador.h descriptador.h setup.h
	$(CXX) $(CXXFLAGS) -o $@ main.cpp $(BRBEPO_SRCS)

benchmark: benchmark.cpp $(BRBEPO_SRCS) openssl_crypto.cpp openssl_crypto.h setup.h
	$(CXX) $(CXXFLAGS) -o $@ benchmark.cpp $(BRBEPO_SRCS) openssl_crypto.cpp $(OPENSSL_LIBS)

clean:
	rm -f programa_BRBEPO benchmark
	rm -f arquivoencriptado.txt arquivodescriptado.txt
