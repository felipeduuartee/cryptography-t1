## Dependencias

```bash
sudo apt install build-essential libssl-dev
```

## Compilar

```bash
make
```

Gera `programa_BRBEPO` e `benchmark`.

## BR-BEPO

Lê `texto.txt`, pede a chave (1 a 25) e gera `arquivoencriptado.txt` e `arquivodescriptado.txt`.

```bash
./programa_BRBEPO
```

## Benchmark

```bash
./benchmark --rodada 25 --saida resultados/benchmark.csv \
    dados_gutenberg/lt1k_73576_excerpt.txt \
    dados_gutenberg/73576-0.txt \
    dados_gutenberg/1065.txt \
    dados_gutenberg/41102-0.txt
```

Resultados: `resultados/benchmark.csv`  
Gráficos: `graficos/grafico_cifragem.png` e `graficos/grafico_decifragem.png`

