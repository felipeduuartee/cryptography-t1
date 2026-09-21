#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path

from preparar_gutenberg import (
    ARQUIVO_TRECHO,
    FONTES,
    criar_trecho_lt1k,
    sha256,
    validar_faixas,
    verificar_fonte,
)


def validar(diretorio: Path) -> None:
    manifesto = diretorio / "manifesto.csv"
    if not manifesto.exists():
        raise FileNotFoundError(f"Manifesto ausente: {manifesto}")

    with manifesto.open(newline="", encoding="utf-8") as f:
        linhas = list(csv.DictReader(f))
    por_arquivo = {x["arquivo"]: x for x in linhas}

    tamanhos: dict[str, int] = {}
    for fonte in FONTES:
        caminho = diretorio / fonte.arquivo
        dados = caminho.read_bytes()
        verificar_fonte(fonte, dados)
        entrada = por_arquivo.get(fonte.arquivo)
        if entrada is None:
            raise ValueError(f"Arquivo nao consta no manifesto: {fonte.arquivo}")
        if int(entrada["tamanho_bytes"]) != len(dados):
            raise ValueError(f"Tamanho divergente do manifesto: {fonte.arquivo}")
        if entrada["sha256"] != sha256(dados):
            raise ValueError(f"SHA-256 divergente do manifesto: {fonte.arquivo}")
        tamanhos[fonte.faixa] = len(dados)

    base = (diretorio / "73576-0.txt").read_bytes()
    trecho = (diretorio / ARQUIVO_TRECHO).read_bytes()
    if trecho != criar_trecho_lt1k(base):
        raise ValueError("O arquivo <1K nao corresponde ao trecho deterministico da fonte.")
    if trecho not in base:
        raise ValueError("O arquivo <1K nao e substring byte a byte da fonte.")
    entrada = por_arquivo.get(ARQUIVO_TRECHO)
    if entrada is None:
        raise ValueError("Trecho <1K nao consta no manifesto.")
    if entrada["sha256"] != hashlib.sha256(trecho).hexdigest():
        raise ValueError("SHA-256 divergente do manifesto para o trecho <1K.")
    tamanhos["<1K"] = len(trecho)

    validar_faixas(tamanhos)
    print("Corpus Gutenberg valido.")
    for faixa in ("<1K", "1K-10K", "10K-100K", ">100K"):
        print(f"  {faixa:>10}: {tamanhos[faixa]} B")


def main() -> None:
    p = argparse.ArgumentParser()
    p.add_argument("--diretorio", default="dados_gutenberg")
    args = p.parse_args()
    validar(Path(args.diretorio))


if __name__ == "__main__":
    main()
