from pathlib import Path
import re

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

OUT_DIR = Path("saidas")
CSV_7Z = OUT_DIR / "benchmark_7z.csv"


def ler_csv(path: Path) -> pd.DataFrame:
    df = pd.read_csv(path, encoding="utf-8-sig")

    df.columns = (
        df.columns.astype(str)
        .str.replace("\ufeff", "", regex=False)
        .str.strip()
    )

    rename_map = {
        "PPM_bytes": "Compressed_bytes",
        "PPM_bps": "BPS",
        "7z_bytes": "Compressed_bytes",
        "7z_bps": "BPS",
        "Orig_bytes": "Original_bytes",
        "Comp_bytes": "Compressed_bytes",
        "Tempo_comp_s": "T_comp_s",
        "Tempo_decomp_s": "T_decomp_s",
    }
    df = df.rename(columns=rename_map)

    for col in ["Arquivo", "Tool", "K", "Status"]:
        if col not in df.columns:
            df[col] = ""

    numeric_cols = [
        "Original_bytes",
        "Compressed_bytes",
        "BPS",
        "T_comp_s",
        "T_decomp_s",
    ]
    for col in numeric_cols:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors="coerce")

    df["Arquivo"] = df["Arquivo"].astype(str).str.strip()
    df["Tool"] = df["Tool"].astype(str).str.strip()
    df["K"] = df["K"].astype(str).str.strip()
    df["Status"] = df["Status"].astype(str).str.strip()

    return df


def normalizar_nome_arquivo(nome: str) -> str:
    nome = str(nome).strip()
    if not nome:
        return nome

    base = Path(nome).name

    while True:
        stem = Path(base).stem
        if stem == base:
            break
        base = stem

    return base.lower()


def carregar_dados() -> pd.DataFrame:
    frames = []

    if CSV_7Z.exists():
        df_7z = ler_csv(CSV_7Z)
        df_7z = df_7z[df_7z["Arquivo"].str.upper() != "TOTAL"].copy()
        df_7z = df_7z[df_7z["Status"].str.upper().isin(["OK", ""])].copy()
        df_7z["Serie"] = "7z-PPMd"
        frames.append(df_7z)
    else:
        print(f"[AVISO] Arquivo nao encontrado: {CSV_7Z}")

    for path in sorted(OUT_DIR.glob("benchmark_ppmc_k*.csv")):
        m = re.search(r"k(\d+)", path.stem.lower())
        if not m:
            continue

        k = m.group(1)
        df = ler_csv(path)
        df = df[df["Arquivo"].str.upper() != "TOTAL"].copy()
        df = df[df["Status"].str.upper().isin(["OK", ""])].copy()
        df["Serie"] = f"PPM-C K={k}"
        frames.append(df)

    if not frames:
        raise FileNotFoundError("Nenhum CSV de benchmark encontrado em 'saidas/'.")

    df_all = pd.concat(frames, ignore_index=True)

    if "BPS" not in df_all.columns and {"Compressed_bytes", "Original_bytes"}.issubset(df_all.columns):
        df_all["BPS"] = (df_all["Compressed_bytes"] * 8) / df_all["Original_bytes"]

    if {"Compressed_bytes", "Original_bytes"}.issubset(df_all.columns):
        df_all["Ratio"] = df_all["Compressed_bytes"] / df_all["Original_bytes"]

    df_all["Arquivo_plot"] = df_all["Arquivo"].apply(normalizar_nome_arquivo)

    return df_all


def ordem_series(cols):
    def chave(nome):
        if nome == "7z-PPMd":
            return (999, nome)
        m = re.search(r"K=(\d+)", nome)
        if m:
            return (int(m.group(1)), nome)
        return (998, nome)

    return sorted(cols, key=chave)


def ordenar_arquivos(index_list):
    return sorted(index_list)


def salvar_fig(fig, nome_arquivo: str):
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    caminho = OUT_DIR / nome_arquivo
    fig.tight_layout()
    fig.savefig(caminho, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"[OK] Grafico salvo em: {caminho}")


def plot_metrica(df: pd.DataFrame, coluna: str, titulo: str, ylabel: str, nome_saida: str, logy: bool = False):
    if coluna not in df.columns:
        print(f"[AVISO] Coluna ausente: {coluna}")
        return

    base = df.dropna(subset=["Arquivo_plot", "Serie", coluna]).copy()

    if base.empty:
        print(f"[AVISO] Sem dados para {coluna}")
        return

    pivot = base.pivot_table(
        index="Arquivo_plot",
        columns="Serie",
        values=coluna,
        aggfunc="mean"
    )

    pivot = pivot.reindex(index=ordenar_arquivos(list(pivot.index)))
    pivot = pivot[ordem_series(list(pivot.columns))]

    fig, ax = plt.subplots(figsize=(16, 7))
    pivot.plot(kind="bar", ax=ax, width=0.85)

    if logy:
        ax.set_yscale("log")
        ax.grid(axis="y", which="both", linestyle="--", alpha=0.35)
    else:
        ax.grid(axis="y", linestyle="--", alpha=0.35)

    ax.set_title(titulo)
    ax.set_xlabel("Arquivo")
    ax.set_ylabel(ylabel)
    ax.tick_params(axis="x", rotation=45)
    ax.legend(title="Serie", fontsize=9)

    salvar_fig(fig, nome_saida)


def main():
    try:
        df = carregar_dados()
    except Exception as e:
        print(f"[ERRO] Falha ao carregar dados: {e}")
        raise

    plot_metrica(
        df,
        coluna="BPS",
        titulo="Bits por simbolo - PPM-C (K=0..4) vs 7z-PPMd",
        ylabel="BPS",
        nome_saida="benchmark_bps_todos.png",
        logy=False
    )

    plot_metrica(
        df,
        coluna="Ratio",
        titulo="Taxa de compressao - PPM-C (K=0..4) vs 7z-PPMd",
        ylabel="Compressed / Original",
        nome_saida="benchmark_ratio_todos.png",
        logy=False
    )

    plot_metrica(
        df,
        coluna="T_comp_s",
        titulo="Tempo de compressao - PPM-C (K=0..4) vs 7z-PPMd",
        ylabel="Tempo (s) - Log",
        nome_saida="benchmark_tcomp_todos.png",
        logy=True
    )

    plot_metrica(
        df,
        coluna="T_decomp_s",
        titulo="Tempo de descompressao - PPM-C (K=0..4) vs 7z-PPMd",
        ylabel="Tempo (s) - Log",
        nome_saida="benchmark_tdecomp_todos.png",
        logy=True
    )


if __name__ == "__main__":
    main()
