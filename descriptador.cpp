#include "descriptador.h"
#include "encriptador.h"
#include "setup.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {
void validar_rodada(int rodada) {
    if (rodada < 1 || rodada > 25) {
        throw std::invalid_argument("A chave deve estar entre 1 e 25.");
    }
}


// faz o contrário do encriptar; monto a lista sem o pipe
std::vector<std::string> separar_tokens(const std::string& texto_cifrado) {
    std::vector<std::string> tokens;
    std::stringstream fluxo(texto_cifrado);
    std::string token;

    while (std::getline(fluxo, token, '|')) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}
}

std::vector<std::string> ler_arquivo_cifrado(const std::string& nome_arquivo)
{
    std::ifstream arquivo(nome_arquivo, std::ios::binary);
    if (!arquivo) {
        throw std::runtime_error("Nao foi possivel abrir o arquivo: " + nome_arquivo);
    }

    std::ostringstream conteudo;
    conteudo << arquivo.rdbuf();
    return separar_tokens(conteudo.str());
}

std::vector<std::string> destranspor_brasileirao(
    const std::vector<std::vector<std::string>>& blocos_cifrados,
    int rodada)
{
    validar_rodada(rodada);

    std::vector<std::string> texto_separado;
    for (const auto& bloco_atual : blocos_cifrados) {
        const int tamanho = static_cast<int>(bloco_atual.size());
        std::vector<std::string> bloco_original(static_cast<size_t>(tamanho));
        int indice_cifrado = 0;

        for (int j = 0; j < 26; ++j) {
            const int posicao_alvo = CHAVES_RODADAS[rodada - 1][j];
            if (posicao_alvo < tamanho) {
                if (indice_cifrado >= tamanho) {
                    throw std::runtime_error("Bloco BR-BEPO cifrado invalido.");
                }
                bloco_original[static_cast<size_t>(posicao_alvo)] =
                    bloco_atual[static_cast<size_t>(indice_cifrado++)];
            }
        }

        if (indice_cifrado != tamanho) {
            throw std::runtime_error("Bloco BR-BEPO cifrado invalido.");
        }

        texto_separado.insert(texto_separado.end(), bloco_original.begin(), bloco_original.end());
    }

    return texto_separado;
}

char reverter_byte_encriptado(const std::string& token)
{
    if (token.size() == 2 && (token[0] == 'U' || token[0] == 'L')) {
        const char letra_cifrada = token[1];
        for (int i = 0; i < 26; ++i) {
            if (BEPO_LAYOUT[i] == letra_cifrada) {
                return static_cast<char>((token[0] == 'U' ? 'A' : 'a') + i);
            }
        }
        throw std::runtime_error("Token BR-BEPO de letra invalido: " + token);
    }

    if (token.size() == 3 && token[0] == 'X') {
        size_t processados = 0;
        const int valor = std::stoi(token.substr(1), &processados, 16);
        if (processados != 2 || valor < 0 || valor > 255) {
            throw std::runtime_error("Token BR-BEPO hexadecimal invalido: " + token);
        }
        return static_cast<char>(static_cast<unsigned char>(valor));
    }

    throw std::runtime_error("Token BR-BEPO invalido: " + token);
}

void arquivo_descriptado(const std::vector<char>& texto_final, const std::string& nome_saida)
{
    std::ofstream arquivo_saida(nome_saida, std::ios::binary);
    if (!arquivo_saida) {
        throw std::runtime_error("Nao foi possivel criar o arquivo: " + nome_saida);
    }

    arquivo_saida.write(texto_final.data(), static_cast<std::streamsize>(texto_final.size()));
}

std::vector<unsigned char> decifrar_brbepo(const std::string& texto_cifrado, int rodada)
{
    validar_rodada(rodada);

    const auto tokens = separar_tokens(texto_cifrado);
    const auto blocos = agrupar_blocos(tokens);
    const auto ordenados = destranspor_brasileirao(blocos, rodada);

    std::vector<unsigned char> resultado;
    resultado.reserve(ordenados.size());
    for (const auto& token : ordenados) {
        resultado.push_back(static_cast<unsigned char>(reverter_byte_encriptado(token)));
    }
    return resultado;
}
