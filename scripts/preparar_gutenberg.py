#!/usr/bin/env python3
"""Baixa e prepara os quatro arquivos do experimento Gutenberg.

A faixa <1K e obtida como um trecho contiguo, byte a byte, de 73576-0.txt.
Os arquivos locais sao registrados no manifesto com tamanho e SHA-256. Para
as duas fontes menores, tambem ha uma verificacao contra snapshots conhecidos.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import urllib.request
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class Fonte:
    faixa: str
    titulo: str
    arquivo: str
    url: str
    tamanho_esperado: int | None
    git_blob_sha1: str | None


FONTES = (
    Fonte(
        "1K-10K",
        "A Kiss for the Conqueror",
        "73576-0.txt",
        "https://www.gutenberg.org/files/73576/73576-0.txt",
        9970,
        "ca80025f242527d2bd4848af272df890dfb3606b",
    ),
    Fonte(
        "10K-100K",
        "The Raven",
        "1065.txt",
        "https://www.gutenberg.org/files/1065/old/old/1065.txt",
        27070,
        "a326a02d21a05b6f69083b0f9a0e07488336133f",
    ),
    Fonte(
        ">100K",
        "The King's Threshold; and On Baile's Strand",
        "41102-0.txt",
        "https://www.gutenberg.org/files/41102/41102-0.txt",
        None,
        None,
    ),
)

ARQUIVO_TRECHO = "lt1k_73576_excerpt.txt"
MARCADOR_TRECHO = b'"Tonight\'s the night," Bolgar said.'


def git_blob_sha1(dados: bytes) -> str:
    cabecalho = f"blob {len(dados)}\0".encode("ascii")
    return hashlib.sha1(cabecalho + dados).hexdigest()


def sha256(dados: bytes) -> str:
    return hashlib.sha256(dados).hexdigest()


def baixar(url: str) -> bytes:
    req = urllib.request.Request(
        url,
        headers={"User-Agent": "BR-BEPO-UFPR-benchmark/1.0"},
    )
    with urllib.request.urlopen(req, timeout=60) as resposta:
        return resposta.read()


def verificar_fonte(fonte: Fonte, dados: bytes) -> None:
    # Quando existe um snapshot conhecido, exige correspondencia byte a byte.
    # Para a fonte >100K, a copia local usada no experimento e registrada pelo
    # proprio SHA-256 do manifesto; o requisito funcional aqui e a faixa de tamanho.
    if fonte.tamanho_esperado is not None and len(dados) != fonte.tamanho_esperado:
        raise ValueError(
            f"{fonte.arquivo}: tamanho {len(dados)} B; esperado {fonte.tamanho_esperado} B."
        )
    if fonte.git_blob_sha1 is not None:
        obtido = git_blob_sha1(dados)
        if obtido != fonte.git_blob_sha1:
            raise ValueError(
                f"{fonte.arquivo}: Git blob SHA-1 divergente: {obtido}; "
                f"esperado {fonte.git_blob_sha1}."
            )


def criar_trecho_lt1k(dados: bytes, limite: int = 1000) -> bytes:
    """Seleciona linhas contiguas da narrativa, sem modificar seus bytes."""
    inicio = dados.find(MARCADOR_TRECHO)
    if inicio < 0:
        raise ValueError("Marcador da narrativa nao encontrado em 73576-0.txt.")

    resto = dados[inicio:]
    trecho = bytearray()
    for linha in resto.splitlines(keepends=True):
        if len(trecho) + len(linha) >= limite:
            break
        trecho.extend(linha)

    if not trecho:
        raise ValueError("Nao foi possivel produzir o trecho <1K.")
    if len(trecho) >= limite:
        raise AssertionError("Trecho produzido fora da faixa <1K decimal.")
    if bytes(trecho) not in dados:
        raise AssertionError("Trecho deixou de ser substring byte a byte da fonte.")
    return bytes(trecho)


def validar_faixas(tamanhos: dict[str, int]) -> None:
    """Usa margens que satisfazem tanto K=1000 quanto KiB=1024."""
    n0 = tamanhos["<1K"]
    n1 = tamanhos["1K-10K"]
    n2 = tamanhos["10K-100K"]
    n3 = tamanhos[">100K"]
    if not (0 < n0 < 1000):
        raise ValueError(f"Faixa <1K invalida: {n0} B")
    if not (1024 < n1 < 10000):
        raise ValueError(f"Faixa 1K-10K invalida/ambigua: {n1} B")
    if not (10240 < n2 < 100000):
        raise ValueError(f"Faixa 10K-100K invalida/ambigua: {n2} B")
    if not (n3 > 102400):
        raise ValueError(f"Faixa >100K invalida/ambigua: {n3} B")


def escrever_manifesto(destino: Path, linhas: list[dict[str, str | int]]) -> None:
    caminho = destino / "manifesto.csv"
    campos = ["faixa", "titulo", "arquivo", "tamanho_bytes", "sha256", "tipo", "url_fonte"]
    with caminho.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=campos)
        w.writeheader()
        w.writerows(linhas)


def preparar(destino: Path, forcar: bool = False) -> None:
    destino.mkdir(parents=True, exist_ok=True)
    dados_por_arquivo: dict[str, bytes] = {}

    for fonte in FONTES:
        caminho = destino / fonte.arquivo
        if caminho.exists() and not forcar:
            dados = caminho.read_bytes()
        else:
            print(f"Baixando {fonte.url}")
            dados = baixar(fonte.url)
            caminho.write_bytes(dados)
        verificar_fonte(fonte, dados)
        dados_por_arquivo[fonte.arquivo] = dados

    base = dados_por_arquivo["73576-0.txt"]
    trecho = criar_trecho_lt1k(base)
    (destino / ARQUIVO_TRECHO).write_bytes(trecho)

    linhas: list[dict[str, str | int]] = [
        {
            "faixa": "<1K",
            "titulo": "A Kiss for the Conqueror (trecho verbatim)",
            "arquivo": ARQUIVO_TRECHO,
            "tamanho_bytes": len(trecho),
            "sha256": sha256(trecho),
            "tipo": "trecho contiguo verbatim autorizado",
            "url_fonte": FONTES[0].url,
        }
    ]
    for fonte in FONTES:
        dados = dados_por_arquivo[fonte.arquivo]
        if fonte.arquivo == "41102-0.txt":
            tipo = "copia local do texto Gutenberg; integridade registrada por SHA-256"
        else:
            tipo = "arquivo completo conferido contra snapshot conhecido"
        linhas.append(
            {
                "faixa": fonte.faixa,
                "titulo": fonte.titulo,
                "arquivo": fonte.arquivo,
                "tamanho_bytes": len(dados),
                "sha256": sha256(dados),
                "tipo": tipo,
                "url_fonte": fonte.url,
            }
        )

    validar_faixas({str(x["faixa"]): int(x["tamanho_bytes"]) for x in linhas})
    escrever_manifesto(destino, linhas)
    print("Corpus preparado e verificado:")
    for linha in linhas:
        print(f"  {linha['faixa']:>10}: {linha['arquivo']} ({linha['tamanho_bytes']} B)")
    print(destino / "manifesto.csv")


def main() -> None:
    p = argparse.ArgumentParser()
    p.add_argument("--diretorio", default="dados_gutenberg")
    p.add_argument("--forcar", action="store_true", help="baixa novamente mesmo se os arquivos existirem")
    args = p.parse_args()
    preparar(Path(args.diretorio), args.forcar)


if __name__ == "__main__":
    main()
