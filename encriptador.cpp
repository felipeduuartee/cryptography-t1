#include "encriptador.h"
#include "setup.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <stdexcept>

namespace {
void validar_rodada(int rodada) {
    if (rodada < 1 || rodada > 25) {
        throw std::invalid_argument("A chave deve estar entre 1 e 25.");
    }
}
}

std::string substituir_byte(char c)
{
    if (c >= 'A' && c <= 'Z') {
        const int indice = c - 'A';
        return std::string("U") + BEPO_LAYOUT[indice];
    }

    if (c >= 'a' && c <= 'z') {
        const int indice = c - 'a';
        return std::string("L") + BEPO_LAYOUT[indice];
    }

    char hex_str[4];
    std::snprintf(hex_str, sizeof(hex_str), "X%02X", static_cast<unsigned char>(c));
    return std::string(hex_str);
}

std::vector<std::string> ler_arquivo(const std::string& nome_arquivo)
{
    std::ifstream arquivo(nome_arquivo, std::ios::binary);
    if (!arquivo) {
        throw std::runtime_error("Nao foi possivel abrir o arquivo: " + nome_arquivo);
    }

    std::vector<std::string> texto_bytes;
    char byte_lido;
    while (arquivo.get(byte_lido)) {
        texto_bytes.push_back(substituir_byte(byte_lido) + "|");
    }
    return texto_bytes;
}

std::vector<std::vector<std::string>> agrupar_blocos(const std::vector<std::string>& texto_bytes)
{
    std::vector<std::vector<std::string>> blocos;
    blocos.reserve((texto_bytes.size() + 25) / 26);

    for (size_t i = 0; i < texto_bytes.size(); i += 26) {
        const size_t fim = std::min(i + 26, texto_bytes.size());
        blocos.emplace_back(texto_bytes.begin() + static_cast<std::ptrdiff_t>(i),
                            texto_bytes.begin() + static_cast<std::ptrdiff_t>(fim));
    }

    return blocos;
}

std::vector<std::string> transpor_brasileirao(
    const std::vector<std::vector<std::string>>& blocos_texto,
    int rodada_chave)
{
    validar_rodada(rodada_chave);

    std::vector<std::string> texto_final;
    for (const auto& bloco_atual : blocos_texto) {
        const int tamanho_bloco = static_cast<int>(bloco_atual.size());
        for (int j = 0; j < 26; ++j) {
            const int posicao_alvo = CHAVES_RODADAS[rodada_chave - 1][j];
            if (posicao_alvo < tamanho_bloco) {
                texto_final.push_back(bloco_atual[static_cast<size_t>(posicao_alvo)]);
            }
        }
    }

    return texto_final;
}

void arquivo_encriptado(const std::vector<std::string>& texto_final, const std::string& nome_saida)
{
    std::ofstream arquivo_saida(nome_saida, std::ios::binary);
    if (!arquivo_saida) {
        throw std::runtime_error("Nao foi possivel criar o arquivo: " + nome_saida);
    }

    for (const auto& token : texto_final) {
        arquivo_saida << token;
    }
}

std::string cifrar_brbepo(const std::vector<unsigned char>& dados, int rodada_chave)
{
    validar_rodada(rodada_chave);

    std::vector<std::string> tokens;
    tokens.reserve(dados.size());
    for (const unsigned char byte : dados) {
        tokens.push_back(substituir_byte(static_cast<char>(byte)) + "|");
    }

    const auto blocos = agrupar_blocos(tokens);
    const auto transposto = transpor_brasileirao(blocos, rodada_chave);

    size_t tamanho_total = 0;
    for (const auto& token : transposto) {
        tamanho_total += token.size();
    }

    std::string resultado;
    resultado.reserve(tamanho_total);
    for (const auto& token : transposto) {
        resultado += token;
    }
    return resultado;
}
