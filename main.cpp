#include "descriptador.h"
#include "encriptador.h"

#include <iostream>
#include <stdexcept>
#include <vector>

int main()
{
    try {
        int rodada = 0;
        std::cout << "Digite a chave BR-BEPO (1 a 25): ";
        std::cin >> rodada;

        while (std::cin.fail() || rodada < 1 || rodada > 25) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Valor invalido. Digite um numero entre 1 e 25: ";
            std::cin >> rodada;
        }

        const auto tokens = ler_arquivo("texto.txt");
        const auto blocos = agrupar_blocos(tokens);
        const auto texto_cifrado = transpor_brasileirao(blocos, rodada);
        arquivo_encriptado(texto_cifrado, "arquivoencriptado.txt");

        const auto tokens_lidos = ler_arquivo_cifrado("arquivoencriptado.txt");
        const auto blocos_cifrados = agrupar_blocos(tokens_lidos);
        const auto tokens_ordenados = destranspor_brasileirao(blocos_cifrados, rodada);

        std::vector<char> texto;
        texto.reserve(tokens_ordenados.size());
        for (const auto& token : tokens_ordenados) {
            texto.push_back(reverter_byte_encriptado(token));
        }
        arquivo_descriptado(texto, "arquivodescriptado.txt");

        std::cout << "Gerados: arquivoencriptado.txt e arquivodescriptado.txt\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << '\n';
        return 1;
    }
}
