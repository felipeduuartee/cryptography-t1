#include "descriptador.h"
#include "encriptador.h"
#include "openssl_crypto.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using Relogio = std::chrono::steady_clock;

struct Configuracao {
    int rodada = 0;
    std::string saida = "resultados/benchmark.csv";
    std::vector<std::string> arquivos;
};

std::string escapar_csv(const std::string& valor) {
    if (valor.find_first_of(",\"\n\r") == std::string::npos) {
        return valor;
    }

    std::string saida = "\"";
    for (char c : valor) {
        if (c == '\"') {
            saida += "\"\"";
        } else {
            saida += c;
        }
    }
    saida += '\"';
    return saida;
}

Bytes ler_bytes(const std::string& caminho) {
    std::ifstream arquivo(caminho, std::ios::binary);
    if (!arquivo) {
        throw std::runtime_error("Nao foi possivel abrir o arquivo: " + caminho);
    }

    arquivo.seekg(0, std::ios::end);
    const std::streamoff tamanho = arquivo.tellg();
    arquivo.seekg(0, std::ios::beg);

    if (tamanho < 0) {
        throw std::runtime_error("Nao foi possivel obter o tamanho do arquivo: " + caminho);
    }

    Bytes dados(static_cast<size_t>(tamanho));
    if (tamanho > 0) {
        arquivo.read(reinterpret_cast<char*>(dados.data()), tamanho);
        if (!arquivo) {
            throw std::runtime_error("Falha ao ler o arquivo: " + caminho);
        }
    }
    return dados;
}

long long ns_decorridos(const Relogio::time_point& inicio, const Relogio::time_point& fim) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(fim - inicio).count();
}

void registrar(std::ofstream& csv,
               const std::string& arquivo,
               size_t tamanho,
               const std::string& algoritmo,
               const std::string& operacao,
               long long tempo_ns) {
    csv << escapar_csv(arquivo) << ','
        << tamanho << ','
        << algoritmo << ','
        << operacao << ','
        << tempo_ns << '\n';
}

Configuracao argumentos(int argc, char** argv) {
    Configuracao cfg;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--rodada") {
            if (++i >= argc) throw std::invalid_argument("Falta o valor de --rodada.");
            cfg.rodada = std::stoi(argv[i]);
        } else if (arg == "--saida") {
            if (++i >= argc) throw std::invalid_argument("Falta o valor de --saida.");
            cfg.saida = argv[i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout
                << "Uso: ./benchmark --rodada N [--saida arquivo.csv] arquivo1 [arquivo2 ...]\n"
                << "  --rodada  chave BR-BEPO de 1 a 25 (obrigatoria)\n"
                << "  --saida   CSV de tempos (padrao: resultados/benchmark.csv)\n";
            std::exit(0);
        } else if (!arg.empty() && arg[0] == '-') {
            throw std::invalid_argument("Opcao desconhecida: " + arg);
        } else {
            cfg.arquivos.push_back(arg);
        }
    }

    if (cfg.rodada < 1 || cfg.rodada > 25) {
        throw std::invalid_argument("--rodada deve estar entre 1 e 25.");
    }
    if (cfg.arquivos.empty()) {
        throw std::invalid_argument("Informe pelo menos um arquivo de entrada.");
    }

    return cfg;
}
}

int main(int argc, char** argv)
{
    try {
        const Configuracao cfg = argumentos(argc, argv);

        const std::filesystem::path caminho_saida(cfg.saida);
        if (caminho_saida.has_parent_path()) {
            std::filesystem::create_directories(caminho_saida.parent_path());
        }

        std::ofstream csv(cfg.saida, std::ios::trunc);
        if (!csv) {
            throw std::runtime_error("Nao foi possivel criar o CSV: " + cfg.saida);
        }
        csv << "arquivo,tamanho_bytes,algoritmo,operacao,tempo_ns\n";

        // Chaves e IV sao preparados antes da medicao dos algoritmos.
        AES256CBC aes;
        RSA2048OAEP rsa;

        for (const auto& caminho : cfg.arquivos) {
            const Bytes original = ler_bytes(caminho); // leitura do arquivo fora da medicao
            std::cout << "Arquivo: " << caminho << " (" << original.size() << " bytes)\n";

            {
                const auto inicio = Relogio::now();
                const std::string cifrado = cifrar_brbepo(original, cfg.rodada);
                const auto fim = Relogio::now();

                const auto inicio_dec = Relogio::now();
                const Bytes recuperado = decifrar_brbepo(cifrado, cfg.rodada);
                const auto fim_dec = Relogio::now();

                if (recuperado != original) {
                    throw std::runtime_error("Falha de round-trip BR-BEPO em: " + caminho);
                }
                registrar(csv, caminho, original.size(), "BR-BEPO", "cifragem",
                          ns_decorridos(inicio, fim));
                registrar(csv, caminho, original.size(), "BR-BEPO", "decifragem",
                          ns_decorridos(inicio_dec, fim_dec));
            }

            {
                const auto inicio = Relogio::now();
                const Bytes cifrado = aes.cifrar(original);
                const auto fim = Relogio::now();

                const auto inicio_dec = Relogio::now();
                const Bytes recuperado = aes.decifrar(cifrado);
                const auto fim_dec = Relogio::now();

                if (recuperado != original) {
                    throw std::runtime_error("Falha de round-trip AES em: " + caminho);
                }
                registrar(csv, caminho, original.size(), "AES-256-CBC", "cifragem",
                          ns_decorridos(inicio, fim));
                registrar(csv, caminho, original.size(), "AES-256-CBC", "decifragem",
                          ns_decorridos(inicio_dec, fim_dec));
            }

            {
                const auto inicio = Relogio::now();
                const Bytes cifrado = rsa.cifrar(original);
                const auto fim = Relogio::now();

                const auto inicio_dec = Relogio::now();
                const Bytes recuperado = rsa.decifrar(cifrado);
                const auto fim_dec = Relogio::now();

                if (recuperado != original) {
                    throw std::runtime_error("Falha de round-trip RSA em: " + caminho);
                }
                registrar(csv, caminho, original.size(), "RSA-2048-OAEP-SHA256", "cifragem",
                          ns_decorridos(inicio, fim));
                registrar(csv, caminho, original.size(), "RSA-2048-OAEP-SHA256", "decifragem",
                          ns_decorridos(inicio_dec, fim_dec));
            }
        }

        std::cout << "Tempos salvos em: " << cfg.saida << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << '\n';
        return 1;
    }
}
