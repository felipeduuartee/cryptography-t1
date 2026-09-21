# BR-BEPO

Implementação da cifra BR-BEPO e comparação de desempenho com AES-256-CBC e RSA-2048-OAEP-SHA256.

O relatório completo está disponível em `BR_BEPO__Do_Brasileirão_aos_Bytes.pdf`.

## Requisitos

- compilador C++ com suporte a C++17;
- GNU Make;
- OpenSSL com headers de desenvolvimento;
- Python 3;
- Matplotlib, somente para gerar os gráficos.

No Ubuntu ou Debian:

```bash
sudo apt update
sudo apt install build-essential libssl-dev python3 python3-venv
```

## Clonar o repositório

```bash
git clone https://github.com/felipeduuartee/cryptography-t1.git
cd cryptography-t1
```

## Compilar

```bash
make
```

São gerados os executáveis:

- `programa_BRBEPO`: demonstração da cifra BR-BEPO;
- `benchmark`: comparação entre BR-BEPO, AES e RSA;
- `gerador_tabela`: reprodução das 25 permutações.

## Executar a BR-BEPO

O programa lê o conteúdo de `texto.txt` e solicita uma chave entre 1 e 25:

```bash
./programa_BRBEPO
```

Arquivos gerados:

- `arquivoencriptado.txt`;
- `arquivodescriptado.txt`.

Para verificar se o arquivo foi recuperado exatamente:

```bash
cmp texto.txt arquivodescriptado.txt \
    && echo "SUCESSO: arquivos idênticos" \
    || echo "FALHA: arquivos diferentes"
```

## Executar as validações

```bash
make validar-chave
make validar-dados
```

## Executar o experimento completo

Crie um ambiente virtual e instale o Matplotlib:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install matplotlib
```

Execute o benchmark e gere o CSV e os gráficos:

```bash
make experimento
```

Os arquivos são gravados em:

```text
resultados/benchmark.csv
resultados/grafico_cifragem.png
resultados/grafico_decifragem.png
```

`make experimento` realiza novas medições e sobrescreve os resultados existentes.

Para recriar somente os gráficos usando o CSV atual:

```bash
make graphs
```

Para sair do ambiente virtual:

```bash
deactivate
```

## Limpar arquivos gerados

```bash
make clean
```
