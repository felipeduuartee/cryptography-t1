#pragma once

#include <string>
#include <vector>

std::vector<std::string> ler_arquivo_cifrado(const std::string& nome_arquivo);
std::vector<std::string> destranspor_brasileirao(const std::vector<std::vector<std::string>>& blocos_cifrados, int rodada);
char reverter_byte_encriptado(const std::string& token);
void arquivo_descriptado(const std::vector<char>& texto_final, const std::string& nome_saida);

std::vector<unsigned char> decifrar_brbepo(const std::string& texto_cifrado, int rodada);
