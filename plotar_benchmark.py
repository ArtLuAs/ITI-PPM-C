import sys
from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

OUT_DIR = Path("saidas")


def preparar_dataframe(csv_path: Path) -> pd.DataFrame:
    if not csv_path.exists():
        raise FileNotFoundError(f"CSV nao encontrado: {csv_path}")

    df = pd.read_csv(csv_path, encoding="utf-8-sig")

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

    if "Arquivo" not in df.columns:
        raise ValueError(
            f"CSV invalido: coluna 'Arquivo' nao encontrada. Colunas atuais: {df.columns.tolist()}"
        )

    if "Tool" not in df.columns:
        if "K" in df.columns:
            k_as_str = df["K"].astype(str).str.strip().str.lower()
            df["Tool"] = np.where(k_as_str.eq("7z"), "7z-PPMd", "PPM-C")
        else:
            df["Tool"] = "PPM-C"

    if "K" not in df.columns:
        df["K"] = ""

    df["Arquivo"] = df["Arquivo"].astype(str).str.strip()
    df["Tool"] = df["Tool"].astype(str).str.strip()
    df["K"] = df["K"].astype(str).str.strip()

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

    if "BPS" not in df.columns and {"Compressed_bytes", "Original_bytes"}.issubset(df.columns):
        df["BPS"] = (df["Compressed_bytes"] * 8) / df["Original_bytes"]

    if "Original_bytes" not in df.columns and {"Compressed_bytes", "BPS"}.issubset(df.columns):
        with np.errstate(divide="ignore", invalid="ignore"):
            df["Original_bytes"] = (df["Compressed_bytes"] * 8) / df["BPS"]

    return df


def salvar_fig(fig, nome_arquivo: str):
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    caminho = OUT_DIR / nome_arquivo
    fig.tight_layout()
    fig.savefig(caminho, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"[OK] Grafico salvo em: {caminho}")


def plot_barras_bps(df: pd.DataFrame, k_alvo: str):
    if "BPS" not in df.columns:
        print("[AVISO] Coluna 'BPS' ausente; pulando grafico de BPS.")
        return

    df_plot = df.copy()

    mask_ppmc = (df_plot["Tool"].str.lower() == "ppm-c") & (df_plot["K"] == k_alvo)
    mask_7z = df_plot["Tool"].str.lower().isin(["7z-ppmd", "7z", "7zip", "7z_ppmd"])
    df_plot = df_plot[mask_ppmc | mask_7z].copy()

    if df_plot.empty:
        print(f"[AVISO] Nenhum dado encontrado para K={k_alvo} no grafico de BPS.")
        return

    pivot = df_plot.pivot_table(
        index="Arquivo",
        columns="Tool",
        values="BPS",
        aggfunc="mean"
    )

    ordem_cols = [c for c in ["PPM-C", "7z-PPMd"] if c in pivot.columns] + \
                 [c for c in pivot.columns if c not in ["PPM-C", "7z-PPMd"]]
    pivot = pivot[ordem_cols]

    fig, ax = plt.subplots(figsize=(12, 6))
    pivot.plot(kind="bar", ax=ax)

    ax.set_title(f"Bits por simbolo - PPM-C (K={k_alvo}) vs 7z-PPMd")
    ax.set_xlabel("Arquivo")
    ax.set_ylabel("BPS")
    ax.tick_params(axis="x", rotation=45)
    ax.grid(axis="y", linestyle="--", alpha=0.4)

    salvar_fig(fig, f"benchmark_bps_k{k_alvo}.png")


def plot_barras_tempo(df: pd.DataFrame, k_alvo: str, coluna_tempo: str, titulo: str, nome_saida: str):
    if coluna_tempo not in df.columns:
        print(f"[AVISO] Coluna '{coluna_tempo}' ausente; pulando grafico.")
        return

    df_plot = df.copy()

    mask_ppmc = (df_plot["Tool"].str.lower() == "ppm-c") & (df_plot["K"] == k_alvo)
    mask_7z = df_plot["Tool"].str.lower().isin(["7z-ppmd", "7z", "7zip", "7z_ppmd"])
    df_plot = df_plot[mask_ppmc | mask_7z].copy()

    if df_plot.empty:
        print(f"[AVISO] Nenhum dado encontrado para K={k_alvo} em {coluna_tempo}.")
        return

    pivot = df_plot.pivot_table(
        index="Arquivo",
        columns="Tool",
        values=coluna_tempo,
        aggfunc="mean"
    )

    ordem_cols = [c for c in ["PPM-C", "7z-PPMd"] if c in pivot.columns] + \
                 [c for c in pivot.columns if c not in ["PPM-C", "7z-PPMd"]]
    pivot = pivot[ordem_cols]

    fig, ax = plt.subplots(figsize=(12, 6))
    pivot.plot(kind="bar", ax=ax)

    ax.set_yscale("log")
    ax.set_title(f"{titulo} - PPM-C (K={k_alvo}) vs 7z-PPMd (Escala Log)")
    ax.set_xlabel("Arquivo")
    ax.set_ylabel("Tempo (s) - Log")
    ax.tick_params(axis="x", rotation=45)
    ax.grid(axis="y", which="both", linestyle="--", alpha=0.4)

    salvar_fig(fig, nome_saida)


def plot_taxa_compressao(df: pd.DataFrame, k_alvo: str):
    if not {"Original_bytes", "Compressed_bytes"}.issubset(df.columns):
        print("[AVISO] Colunas 'Original_bytes'/'Compressed_bytes' ausentes; pulando taxa de compressao.")
        return

    df_plot = df.copy()
    df_plot = df_plot.dropna(subset=["Original_bytes", "Compressed_bytes"]).copy()
    df_plot = df_plot[df_plot["Original_bytes"] > 0].copy()
    df_plot["Ratio"] = df_plot["Compressed_bytes"] / df_plot["Original_bytes"]

    mask_ppmc = (df_plot["Tool"].str.lower() == "ppm-c") & (df_plot["K"] == k_alvo)
    mask_7z = df_plot["Tool"].str.lower().isin(["7z-ppmd", "7z", "7zip", "7z_ppmd"])
    df_plot = df_plot[mask_ppmc | mask_7z].copy()

    if df_plot.empty:
        print(f"[AVISO] Nenhum dado encontrado para K={k_alvo} no grafico de taxa.")
        return

    pivot = df_plot.pivot_table(
        index="Arquivo",
        columns="Tool",
        values="Ratio",
        aggfunc="mean"
    )

    ordem_cols = [c for c in ["PPM-C", "7z-PPMd"] if c in pivot.columns] + \
                 [c for c in pivot.columns if c not in ["PPM-C", "7z-PPMd"]]
    pivot = pivot[ordem_cols]

    fig, ax = plt.subplots(figsize=(12, 6))
    pivot.plot(kind="bar", ax=ax)

    ax.set_title(f"Taxa de compressao - PPM-C (K={k_alvo}) vs 7z-PPMd")
    ax.set_xlabel("Arquivo")
    ax.set_ylabel("Compressed / Original")
    ax.tick_params(axis="x", rotation=45)
    ax.grid(axis="y", linestyle="--", alpha=0.4)

    salvar_fig(fig, f"benchmark_ratio_k{k_alvo}.png")


def plot_resumo_console(df: pd.DataFrame, k_alvo: str):
    print("\n=== COLUNAS LIDAS ===")
    print(df.columns.tolist())

    print("\n=== AMOSTRA ===")
    print(df.head(10).to_string(index=False))

    subset = df.copy()
    mask_ppmc = (subset["Tool"].str.lower() == "ppm-c") & (subset["K"] == k_alvo)
    mask_7z = subset["Tool"].str.lower().isin(["7z-ppmd", "7z", "7zip", "7z_ppmd"])
    subset = subset[mask_ppmc | mask_7z].copy()

    if subset.empty:
        print(f"\n[AVISO] Nenhuma linha encontrada para K={k_alvo}.")
        return

    colunas_resumo = [c for c in ["Arquivo", "Tool", "K", "Original_bytes", "Compressed_bytes", "BPS", "T_comp_s", "T_decomp_s"] if c in subset.columns]

    print(f"\n=== RESUMO FILTRADO K={k_alvo} ===")
    print(subset[colunas_resumo].head(20).to_string(index=False))


def main():
    k_alvo = sys.argv[1] if len(sys.argv) > 1 else "4"
    csv_path = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("saidas/benchmark_resultados.csv")

    try:
        df = preparar_dataframe(csv_path)
    except Exception as e:
        print(f"[ERRO] Falha ao preparar dataframe: {e}")
        sys.exit(1)

    plot_resumo_console(df, k_alvo)

    try:
        plot_barras_bps(df, k_alvo)
        plot_taxa_compressao(df, k_alvo)
        plot_barras_tempo(
            df, k_alvo,
            "T_comp_s",
            "Tempo de compressao",
            f"benchmark_tcomp_k{k_alvo}.png"
        )
        plot_barras_tempo(
            df, k_alvo,
            "T_decomp_s",
            "Tempo de descompressao",
            f"benchmark_tdecomp_k{k_alvo}.png"
        )
    except Exception as e:
        print(f"[ERRO] ao gerar os graficos com K={k_alvo}: {e}")
        raise


if __name__ == "__main__":
    main()
