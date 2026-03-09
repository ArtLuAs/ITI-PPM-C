from pathlib import Path
import pandas as pd
import numpy as np

OUT_DIR = Path("saidas")
CSV_7Z = OUT_DIR / "benchmark_7z.csv"
K_VALUES = [0, 1, 2, 3, 4]

def ler_csv(path: Path) -> pd.DataFrame:
    df = pd.read_csv(path, encoding="utf-8-sig")

    df.columns = (
        df.columns.astype(str)
        .str.replace("\ufeff", "", regex=False)
        .str.strip()
    )

    for col in ["Arquivo", "Tool", "K", "Status"]:
        if col not in df.columns:
            df[col] = ""

    for col in ["Original_bytes", "Compressed_bytes", "BPS", "T_comp_s", "T_decomp_s"]:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors="coerce")

    df["Arquivo"] = df["Arquivo"].astype(str).str.strip()
    df["Tool"] = df["Tool"].astype(str).str.strip()
    df["K"] = df["K"].astype(str).str.strip()
    df["Status"] = df["Status"].astype(str).str.strip()

    return df

def pegar_total(df: pd.DataFrame, tool: str):
    subset = df.copy()
    subset = subset[subset["Arquivo"].str.upper() == "TOTAL"]
    subset = subset[subset["Tool"].str.lower() == tool.lower()]
    subset = subset[subset["Status"].str.upper().isin(["OK", ""])]

    if subset.empty:
        return None
    return subset.iloc[0]

def fmt(x, casas=6):
    if pd.isna(x):
        return ""
    return f"{float(x):.{casas}f}"

def main():
    if not CSV_7Z.exists():
        print(f"[ERRO] Arquivo de referencia 7z nao encontrado: {CSV_7Z}")
        return

    df_7z = ler_csv(CSV_7Z)
    total_7z = pegar_total(df_7z, "7z-PPMd")

    if total_7z is None:
        print("[ERRO] Linha TOTAL do 7z-PPMd nao encontrada.")
        return

    linhas = []

    for k in K_VALUES:
        path = OUT_DIR / f"benchmark_ppmc_k{k}.csv"
        if not path.exists():
            print(f"[AVISO] Arquivo nao encontrado: {path}")
            continue

        df = ler_csv(path)
        total_ppmc = pegar_total(df, "PPM-C")

        if total_ppmc is None:
            print(f"[AVISO] Linha TOTAL do PPM-C nao encontrada em K={k}")
            continue

        linhas.append({
            "Kmax": k,
            "BPS_final_PPM-C": total_ppmc.get("BPS", np.nan),
            "Tempo_comp_total_s_PPM-C": total_ppmc.get("T_comp_s", np.nan),
            "Tempo_decomp_total_s_PPM-C": total_ppmc.get("T_decomp_s", np.nan),
            "BPS_7z": total_7z.get("BPS", np.nan),
            "Tempo_comp_total_s_7z": total_7z.get("T_comp_s", np.nan),
            "Tempo_decomp_total_s_7z": total_7z.get("T_decomp_s", np.nan),
            "Obs": ""
        })

    if not linhas:
        print("[ERRO] Nenhum resultado de PPM-C encontrado.")
        return

    tabela = pd.DataFrame(linhas).sort_values("Kmax").reset_index(drop=True)

    idx_melhor = tabela["BPS_final_PPM-C"].astype(float).idxmin()
    tabela.loc[idx_melhor, "Obs"] = "Melhor BPS entre os K testados"

    csv_saida = OUT_DIR / "tabela_consolidada_k0_a_k4.csv"
    md_saida = OUT_DIR / "tabela_consolidada_k0_a_k4.md"

    tabela.to_csv(csv_saida, index=False, encoding="utf-8-sig")

    tabela_md = tabela.copy()
    for col in [
        "BPS_final_PPM-C",
        "Tempo_comp_total_s_PPM-C",
        "Tempo_decomp_total_s_PPM-C",
        "BPS_7z",
        "Tempo_comp_total_s_7z",
        "Tempo_decomp_total_s_7z",
    ]:
        tabela_md[col] = tabela_md[col].apply(fmt)

    markdown = tabela_md.to_markdown(index=False)

    with open(md_saida, "w", encoding="utf-8") as f:
        f.write(markdown + "\n")

    print(f"[OK] CSV salvo em: {csv_saida}")
    print(f"[OK] Markdown salvo em: {md_saida}")
    print("\n=== TABELA CONSOLIDADA ===")
    print(markdown)

if __name__ == "__main__":
    main()
