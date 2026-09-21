#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include <map>

// Estrutura para acompanhar a pontuacao de cada time
struct Time {
    int id;
    std::string nome;
    int pontos;
    int vitorias;
    int saldo_gols;
    int gols_pro;
};

// Criterios de ordenacao usados pelo gerador
bool ordenarTabela(const Time& a, const Time& b) {
    if (a.pontos != b.pontos) return a.pontos > b.pontos;
    if (a.vitorias != b.vitorias) return a.vitorias > b.vitorias;
    if (a.saldo_gols != b.saldo_gols) return a.saldo_gols > b.saldo_gols;
    if (a.gols_pro != b.gols_pro) return a.gols_pro > b.gols_pro;
    return a.nome < b.nome; // Ordem alfabetica em ultimo caso
}

// Funcao auxiliar para remover espacos extras dos nomes
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

int main() {
    std::ifstream arquivo("resultados_brasileirao_2002.txt");
    if (!arquivo.is_open()) {
        std::cerr << "Erro ao abrir o arquivo resultados_brasileirao_2002.txt" << std::endl;
        return 1;
    }

    std::map<std::string, Time> times;
    std::vector<std::vector<int>> matriz_campeonato;
    int next_id = 0;
    int partidas_lidas = 0;

    // Expressao regular para capturar "Time A  X-Y  Time B"
    // Pega tudo antes do numero como Time A, e tudo depois do traco/numero como Time B
    std::regex regex_placar(R"(^\s*([a-zA-ZÀ-ÿ\-\s]+?)\s+(\d+)\-(\d+)\s+([a-zA-ZÀ-ÿ\-\s]+?)(?:\s+\[.*\])?$)");
    std::string linha;

    while (std::getline(arquivo, linha)) {
        std::smatch match;
        
        // Se a linha contiver o padrao de um placar
        if (std::regex_search(linha, match, regex_placar)) {
            std::string time1 = trim(match[1].str());
            int gols1 = std::stoi(match[2].str());
            int gols2 = std::stoi(match[3].str());
            std::string time2 = trim(match[4].str());

            // Registra os times no mapa se ainda nao existirem
            if (times.find(time1) == times.end()) times[time1] = {next_id++, time1, 0, 0, 0, 0};
            if (times.find(time2) == times.end()) times[time2] = {next_id++, time2, 0, 0, 0, 0};

            // Atualiza estatisticas Time 1
            times[time1].gols_pro += gols1;
            times[time1].saldo_gols += (gols1 - gols2);
            // Atualiza estatisticas Time 2
            times[time2].gols_pro += gols2;
            times[time2].saldo_gols += (gols2 - gols1);

            // Calcula Pontos e Vitorias
            if (gols1 > gols2) {
                times[time1].pontos += 3;
                times[time1].vitorias += 1;
            } else if (gols2 > gols1) {
                times[time2].pontos += 3;
                times[time2].vitorias += 1;
            } else {
                times[time1].pontos += 1;
                times[time2].pontos += 1;
            }

            partidas_lidas++;

            // A cada 13 partidas (1 rodada completa para 26 times), fecha a classificacao
            if (partidas_lidas % 13 == 0) {
                std::vector<Time> tabela_atual;
                for (auto const& par : times) {
                    tabela_atual.push_back(par.second);
                }

                // Ordena seguindo as regras do campeonato
                std::sort(tabela_atual.begin(), tabela_atual.end(), ordenarTabela);

                // Salva a ordem dos IDs na matriz
                std::vector<int> rodada_ids;
                for (const auto& t : tabela_atual) {
                    rodada_ids.push_back(t.id);
                }
                matriz_campeonato.push_back(rodada_ids);
                
                // Limita a primeira fase a 325 partidas (25 blocos de 13)
                if (matriz_campeonato.size() == 25) break; 
            }
        }
    }

    // Gera a saida formatada em C++
    std::cout << "int campeonato[25][26] = {\n";
    for (size_t r = 0; r < matriz_campeonato.size(); ++r) {
        std::cout << "    {";
        for (size_t t = 0; t < matriz_campeonato[r].size(); ++t) {
            std::cout << matriz_campeonato[r][t] << (t < 25 ? ", " : "");
        }
        std::cout << "}" << (r < 24 ? ",\n" : "\n");
    }
    std::cout << "};\n";

    return 0;
}