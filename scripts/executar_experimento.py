#!/usr/bin/env python3
from __future__ import annotations

import csv
import subprocess
from pathlib import Path

ARQUIVOS = [
    "lt1k_73576_excerpt.txt",
    "73576-0.txt",
    "1065.txt",
    "41102-0.txt",
]
ALGORITMOS = {"BR-BEPO", "AES-256-CBC", "RSA-2048-OAEP-SHA256"}
OPERACOES = {"cifragem", "decifragem"}


def main() -> None:
    dados = Path("dados_gutenberg")
    resultados = Path("resultados")
    resultados.mkdir(parents=True, exist_ok=True)

    subprocess.run(["python3", "scripts/validar_gutenberg.py", "--diretorio", str(dados)], check=True)
    subprocess.run(["make", "benchmark"], check=True)

    csv_saida = resultados / "benchmark.csv"
    arquivos = [str(dados / nome) for nome in ARQUIVOS]
    subprocess.run(
        ["./benchmark", "--rodada", "25", "--saida", str(csv_saida), *arquivos],
        check=True,
    )

    with csv_saida.open(newline="", encoding="utf-8") as f:
        linhas = list(csv.DictReader(f))

    combinacoes = {
        (linha["arquivo"], linha["algoritmo"], linha["operacao"])
        for linha in linhas
    }
    esperadas = {
        (str(dados / arquivo), algoritmo, operacao)
        for arquivo in ARQUIVOS
        for algoritmo in ALGORITMOS
        for operacao in OPERACOES
    }
    if combinacoes != esperadas or len(linhas) != len(esperadas):
        faltantes = sorted(esperadas - combinacoes)
        extras = sorted(combinacoes - esperadas)
        raise ValueError(f"CSV incompleto ou duplicado. Faltantes={faltantes}; extras={extras}")

    subprocess.run(
        ["python3", "scripts/plot_benchmark.py", str(csv_saida), "--diretorio", str(resultados)],
        check=True,
    )

    print("Experimento concluido.")
    print(csv_saida)
    print(resultados / "grafico_cifragem.png")
    print(resultados / "grafico_decifragem.png")


if __name__ == "__main__":
    main()
