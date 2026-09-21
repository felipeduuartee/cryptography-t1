# BR-BEPO

Trabalho de Criptografia que implementa a cifra própria **BR-BEPO**, combinando substituição e transposição, e compara seus tempos de cifragem e decifragem com **AES-256-CBC** e **RSA-2048/OAEP-SHA256** da OpenSSL.

## Visão geral

O projeto possui três partes principais:

1. **BR-BEPO**: cifra e decifra arquivos usando uma chave de 1 a 25.
2. **Benchmark**: mede o tempo de cifragem e decifragem da BR-BEPO, AES e RSA.
3. **Experimento**: executa o benchmark nos quatro arquivos exigidos no trabalho e gera o CSV e os gráficos finais.

A BR-BEPO trabalha byte a byte. Letras ASCII são substituídas com base no arranjo `BPOVDLJZWAUIECTSRNMYXKQGHF`; os demais bytes são representados em hexadecimal. Depois, os tokens são agrupados em blocos de até 26 elementos e transpostos por uma das 25 permutações armazenadas em `setup.h`. Essas permutações foram obtidas a partir dos resultados do Campeonato Brasileiro de 2002.

## Estrutura principal

```text
.
├── main.cpp                     # demonstração da BR-BEPO
├── encriptador.cpp/.h           # cifragem BR-BEPO
├── descriptador.cpp/.h          # decifragem BR-BEPO
├── setup.h                      # layout BEPO e 25 permutações
├── gerador_tabela.cpp           # reproduz as 25 permutações
├── resultados_brasileirao_2002.txt
├── openssl_crypto.cpp/.h        # AES e RSA com OpenSSL
├── benchmark.cpp                # mede os três algoritmos
├── dados_gutenberg/             # quatro arquivos do experimento
├── scripts/                     # preparação, validação, execução e gráficos
└── resultados/                  # CSV e gráficos do experimento
```

## Dependências

- compilador com suporte a C++17;
- GNU Make;
- OpenSSL com headers de desenvolvimento;
- Python 3;
- Matplotlib.

## Ordem recomendada

```bash
make
make experimento
```

As duas etapas têm funções diferentes e estão descritas abaixo.

## 1. Compilação

```bash
make
```

Compila três executáveis:

```text
programa_BRBEPO
benchmark
gerador_tabela
```

- `programa_BRBEPO`: demonstra cifragem e decifragem com a BR-BEPO;
- `benchmark`: executa e cronometra BR-BEPO, AES e RSA;
- `gerador_tabela`: reproduz as 25 permutações a partir dos resultados do Brasileirão de 2002.

## 2. Experimento

```bash
make experimento
```

Esse comando executa o experimento usado para a comparação de desempenho.

A sequência é:

1. compila `benchmark`, caso necessário;
2. valida os quatro arquivos de `dados_gutenberg/`;
3. executa `benchmark` com a **chave BR-BEPO 25**;
4. para cada arquivo, mede uma cifragem e uma decifragem com:
   - BR-BEPO;
   - AES-256-CBC;
   - RSA-2048/OAEP-SHA256;
5. verifica se o CSV contém todas as 24 combinações esperadas;
6. gera os gráficos de cifragem e decifragem.

São quatro arquivos, três algoritmos e duas operações:

```text
4 arquivos × 3 algoritmos × 2 operações = 24 medições
```

Os arquivos utilizados são:

| Faixa | Arquivo | Tamanho |
|---|---|---:|
| `<1K` | `lt1k_73576_excerpt.txt` | 984 B |
| `1K-10K` | `73576-0.txt` | 9.970 B |
| `10K-100K` | `1065.txt` | 27.070 B |
| `>100K` | `41102-0.txt` | 107.810 B |

Ao terminar, são gerados ou substituídos:

```text
resultados/
├── benchmark.csv
├── grafico_cifragem.png
└── grafico_decifragem.png
```

O CSV possui o formato:

```text
arquivo,tamanho_bytes,algoritmo,operacao,tempo_ns
```

### Importante

`make experimento` realiza **novas medições de tempo**. Portanto, cada nova execução sobrescreve `resultados/benchmark.csv` e os dois gráficos. Os valores podem variar de uma execução para outra conforme a máquina e o estado do sistema.

Os resultados versionados no repositório correspondem à execução utilizada no relatório.

## Execução manual da BR-BEPO

Para testar apenas a cifra própria, coloque o conteúdo desejado em:

```text
texto.txt
```

Depois execute:

```bash
./programa_BRBEPO
```

O programa solicita uma chave entre 1 e 25 e cria:

```text
arquivoencriptado.txt
arquivodescriptado.txt
```

`arquivodescriptado.txt` deve reproduzir byte a byte o conteúdo original de `texto.txt`.

## Corpus

Os arquivos do experimento ficam em:

```text
dados_gutenberg/
```

O arquivo:

```text
dados_gutenberg/manifesto.csv
```

registra faixa, título, nome do arquivo, tamanho, SHA-256 e URL de origem.

Para apenas validar o corpus existente:

```bash
make validar-dados
```

Para verificar que `gerador_tabela.cpp` reproduz exatamente as 25 permutações
armazenadas em `setup.h`:

```bash
make validar-chave
```

Para preparar novamente o corpus a partir dos arquivos locais disponíveis:

```bash
make dados
```

## Gráficos

Para recriar somente os gráficos usando o `benchmark.csv` já existente, sem executar novas medições:

```bash
make graphs
```

## Limpeza

```bash
make clean
```

Remove os executáveis compilados e os arquivos temporários produzidos pela demonstração da BR-BEPO. Os dados do corpus e os resultados finais não são removidos.
