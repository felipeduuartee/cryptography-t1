#!/usr/bin/env python3
import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt

ALGORITMOS = ["BR-BEPO", "AES-256-CBC", "RSA-2048-OAEP-SHA256"]
FAIXAS = ["<1K", "1K–10K", "10K–100K", ">100K"]


def carregar(caminho):
    tempos = {}
    tamanhos = {}
    with open(caminho, newline="", encoding="utf-8") as f:
        for linha in csv.DictReader(f):
            arquivo = linha["arquivo"]
            algoritmo = linha["algoritmo"]
            operacao = linha["operacao"]
            tamanho = int(linha["tamanho_bytes"])
            chave = (arquivo, algoritmo, operacao)
            if chave in tempos:
                raise ValueError(f"Medicao duplicada no CSV: {arquivo}, {algoritmo}, {operacao}")
            tempos[chave] = int(linha["tempo_ns"]) / 1_000_000.0
            tamanhos[arquivo] = tamanho
    return tempos, tamanhos


def gerar(tempos, tamanhos, operacao, saida):
    arquivos = sorted(tamanhos, key=lambda a: tamanhos[a])
    if len(arquivos) != 4:
        raise ValueError(f"Esperados quatro arquivos de teste; encontrados {len(arquivos)}.")

    x = list(range(4))
    for algoritmo in ALGORITMOS:
        valores = []
        for arquivo in arquivos:
            chave = (arquivo, algoritmo, operacao)
            if chave not in tempos:
                raise ValueError(f"Falta medicao: {arquivo}, {algoritmo}, {operacao}")
            valores.append(tempos[chave])
        plt.plot(x, valores, marker="o", label=algoritmo)

    rotulos = [f"{faixa}\n{tamanhos[arquivo]} B" for faixa, arquivo in zip(FAIXAS, arquivos)]
    plt.xticks(x, rotulos)
    plt.xlabel("Tamanho dos arquivos")
    plt.ylabel("Tempo (ms)")
    plt.title(f"Tempo de {operacao}")
    plt.legend()
    plt.tight_layout()
    plt.savefig(saida, dpi=180)
    plt.close()


def main():
    p = argparse.ArgumentParser()
    p.add_argument("csv", help="CSV produzido pelo executavel benchmark")
    p.add_argument("--diretorio", default="resultados", help="diretorio de saida")
    args = p.parse_args()

    tempos, tamanhos = carregar(args.csv)
    destino = Path(args.diretorio)
    destino.mkdir(parents=True, exist_ok=True)

    gerar(tempos, tamanhos, "cifragem", destino / "grafico_cifragem.png")
    gerar(tempos, tamanhos, "decifragem", destino / "grafico_decifragem.png")
    print(destino / "grafico_cifragem.png")
    print(destino / "grafico_decifragem.png")


if __name__ == "__main__":
    main()
