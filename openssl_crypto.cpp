#include "openssl_crypto.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>

#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>

namespace {
std::string erro_openssl(const std::string& contexto) {
    const unsigned long codigo = ERR_get_error();
    if (codigo == 0) {
        return contexto;
    }

    char buffer[256];
    ERR_error_string_n(codigo, buffer, sizeof(buffer));
    return contexto + ": " + buffer;
}

void exigir(bool condicao, const std::string& contexto) {
    if (!condicao) {
        throw std::runtime_error(erro_openssl(contexto));
    }
}

EVP_PKEY_CTX* novo_ctx_rsa(EVP_PKEY* chave) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(chave, nullptr);
    exigir(ctx != nullptr, "Falha ao criar contexto RSA");
    return ctx;
}

void configurar_oaep_sha256(EVP_PKEY_CTX* ctx) {
    exigir(EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) > 0,
           "Falha ao configurar RSA-OAEP");
    exigir(EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) > 0,
           "Falha ao configurar SHA-256 no OAEP");
    exigir(EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) > 0,
           "Falha ao configurar SHA-256 no MGF1");
}
}

AES256CBC::AES256CBC()
{
    exigir(RAND_bytes(chave_.data(), static_cast<int>(chave_.size())) == 1,
           "Falha ao gerar chave AES");
    exigir(RAND_bytes(iv_.data(), static_cast<int>(iv_.size())) == 1,
           "Falha ao gerar IV AES");
}

Bytes AES256CBC::cifrar(const Bytes& dados) const
{
    EVP_CIPHER_CTX* bruto = EVP_CIPHER_CTX_new();
    exigir(bruto != nullptr, "Falha ao criar contexto AES");
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx(bruto, EVP_CIPHER_CTX_free);

    exigir(EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, chave_.data(), iv_.data()) == 1,
           "Falha ao inicializar AES-256-CBC");

    Bytes saida(dados.size() + EVP_MAX_BLOCK_LENGTH);
    int escritos = 0;
    int finais = 0;

    if (!dados.empty()) {
        if (dados.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
            throw std::runtime_error("Entrada grande demais para uma chamada EVP AES.");
        }
        exigir(EVP_EncryptUpdate(ctx.get(), saida.data(), &escritos,
                                 dados.data(), static_cast<int>(dados.size())) == 1,
               "Falha durante cifragem AES");
    }

    exigir(EVP_EncryptFinal_ex(ctx.get(), saida.data() + escritos, &finais) == 1,
           "Falha ao finalizar cifragem AES");
    saida.resize(static_cast<size_t>(escritos + finais));
    return saida;
}

Bytes AES256CBC::decifrar(const Bytes& dados_cifrados) const
{
    if (dados_cifrados.empty() || dados_cifrados.size() % 16 != 0) {
        throw std::runtime_error("Ciphertext AES-256-CBC invalido.");
    }
    if (dados_cifrados.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
        throw std::runtime_error("Entrada grande demais para uma chamada EVP AES.");
    }

    EVP_CIPHER_CTX* bruto = EVP_CIPHER_CTX_new();
    exigir(bruto != nullptr, "Falha ao criar contexto AES");
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx(bruto, EVP_CIPHER_CTX_free);

    exigir(EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, chave_.data(), iv_.data()) == 1,
           "Falha ao inicializar AES-256-CBC");

    Bytes saida(dados_cifrados.size());
    int escritos = 0;
    int finais = 0;

    exigir(EVP_DecryptUpdate(ctx.get(), saida.data(), &escritos,
                             dados_cifrados.data(), static_cast<int>(dados_cifrados.size())) == 1,
           "Falha durante decifragem AES");
    exigir(EVP_DecryptFinal_ex(ctx.get(), saida.data() + escritos, &finais) == 1,
           "Falha ao finalizar decifragem AES");

    saida.resize(static_cast<size_t>(escritos + finais));
    return saida;
}

RSA2048OAEP::RSA2048OAEP()
{
    EVP_PKEY_CTX* bruto = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    exigir(bruto != nullptr, "Falha ao criar contexto de geracao RSA");
    std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> ctx(bruto, EVP_PKEY_CTX_free);

    exigir(EVP_PKEY_keygen_init(ctx.get()) > 0, "Falha ao inicializar geracao RSA");
    exigir(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), 2048) > 0,
           "Falha ao configurar RSA-2048");
    exigir(EVP_PKEY_keygen(ctx.get(), &chave_) > 0, "Falha ao gerar chave RSA-2048");
}

RSA2048OAEP::~RSA2048OAEP()
{
    EVP_PKEY_free(chave_);
}

Bytes RSA2048OAEP::cifrar(const Bytes& dados) const
{
    if (dados.empty()) {
        return {};
    }

    const size_t quantidade_blocos = (dados.size() + TAMANHO_MAX_BLOCO - 1) / TAMANHO_MAX_BLOCO;
    Bytes saida;
    saida.reserve(quantidade_blocos * TAMANHO_CHAVE_BYTES);

    for (size_t inicio = 0; inicio < dados.size(); inicio += TAMANHO_MAX_BLOCO) {
        const size_t tamanho = std::min(TAMANHO_MAX_BLOCO, dados.size() - inicio);

        EVP_PKEY_CTX* bruto = novo_ctx_rsa(chave_);
        std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> ctx(bruto, EVP_PKEY_CTX_free);
        exigir(EVP_PKEY_encrypt_init(ctx.get()) > 0, "Falha ao inicializar cifragem RSA");
        configurar_oaep_sha256(ctx.get());

        size_t tamanho_saida = 0;
        exigir(EVP_PKEY_encrypt(ctx.get(), nullptr, &tamanho_saida,
                                dados.data() + inicio, tamanho) > 0,
               "Falha ao calcular tamanho da saida RSA");

        const size_t posicao_saida = saida.size();
        saida.resize(posicao_saida + tamanho_saida);
        exigir(EVP_PKEY_encrypt(ctx.get(), saida.data() + posicao_saida, &tamanho_saida,
                                dados.data() + inicio, tamanho) > 0,
               "Falha durante cifragem RSA");
        saida.resize(posicao_saida + tamanho_saida);
    }

    return saida;
}

Bytes RSA2048OAEP::decifrar(const Bytes& dados_cifrados) const
{
    if (dados_cifrados.empty()) {
        return {};
    }
    if (dados_cifrados.size() % TAMANHO_CHAVE_BYTES != 0) {
        throw std::runtime_error("Ciphertext RSA invalido: tamanho nao multiplo de 256 bytes.");
    }

    Bytes saida;
    saida.reserve((dados_cifrados.size() / TAMANHO_CHAVE_BYTES) * TAMANHO_MAX_BLOCO);

    for (size_t inicio = 0; inicio < dados_cifrados.size(); inicio += TAMANHO_CHAVE_BYTES) {
        EVP_PKEY_CTX* bruto = novo_ctx_rsa(chave_);
        std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> ctx(bruto, EVP_PKEY_CTX_free);
        exigir(EVP_PKEY_decrypt_init(ctx.get()) > 0, "Falha ao inicializar decifragem RSA");
        configurar_oaep_sha256(ctx.get());

        size_t tamanho_saida = 0;
        exigir(EVP_PKEY_decrypt(ctx.get(), nullptr, &tamanho_saida,
                                dados_cifrados.data() + inicio, TAMANHO_CHAVE_BYTES) > 0,
               "Falha ao calcular tamanho da saida RSA");

        const size_t posicao_saida = saida.size();
        saida.resize(posicao_saida + tamanho_saida);
        exigir(EVP_PKEY_decrypt(ctx.get(), saida.data() + posicao_saida, &tamanho_saida,
                                dados_cifrados.data() + inicio, TAMANHO_CHAVE_BYTES) > 0,
               "Falha durante decifragem RSA");
        saida.resize(posicao_saida + tamanho_saida);
    }

    return saida;
}
