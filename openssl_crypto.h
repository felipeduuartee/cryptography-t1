#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <vector>

#include <openssl/evp.h>

using Bytes = std::vector<unsigned char>;

class AES256CBC {
public:
    AES256CBC();

    Bytes cifrar(const Bytes& dados) const;
    Bytes decifrar(const Bytes& dados_cifrados) const;

private:
    std::array<unsigned char, 32> chave_{};
    std::array<unsigned char, 16> iv_{};
};

class RSA2048OAEP {
public:
    static constexpr std::size_t TAMANHO_CHAVE_BYTES = 256;
    static constexpr std::size_t TAMANHO_MAX_BLOCO = 190;

    RSA2048OAEP();
    ~RSA2048OAEP();

    RSA2048OAEP(const RSA2048OAEP&) = delete;
    RSA2048OAEP& operator=(const RSA2048OAEP&) = delete;

    Bytes cifrar(const Bytes& dados) const;
    Bytes decifrar(const Bytes& dados_cifrados) const;

private:
    EVP_PKEY* chave_ = nullptr;
};
