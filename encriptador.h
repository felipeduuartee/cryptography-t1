#pragma once

#include <string>
#include <vector>

std::string substituir_byte(char c);
std::vector<std::string> ler_arquivo(const std::string& nome_arquivo);
std::vector<std::vector<std::string>> agrupar_blocos(const std::vector<std::string>& texto_bytes);
std::vector<std::string> transpor_brasileirao(const std::vector<std::vector<std::string>>& blocos_texto, int rodada_chave);
void arquivo_encriptado(const std::vector<std::string>& texto_final, const std::string& nome_saida);

std::string cifrar_brbepo(const std::vector<unsigned char>& dados, int rodada_chave);
