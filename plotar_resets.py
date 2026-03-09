import pandas as pd
import matplotlib.pyplot as plt
import sys

arquivo_csv = "saidas/dickens.bin_grafico.csv"

df = pd.read_csv(arquivo_csv)

# Separa as linhas de reset
resets = df[df["ResetFlag"] == 1]

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 10), sharex=True)

# --- GRÁFICO 1: Curvas de BPS e o threshold ---
ax1.plot(df["BytesProcessados"], df["L_Barra_Janela"],  color="cyan",   alpha=0.4, linewidth=1,  label="L_Janela (BPS bruto)")
ax1.plot(df["BytesProcessados"], df["SmoothedBPS"],     color="blue",   linewidth=2,              label="SmoothedBPS (média móvel)")
ax1.plot(df["BytesProcessados"], df["ThresholdBPS"],    color="orange", linewidth=1.5, linestyle="--", label="Threshold (+50%)")
ax1.plot(df["BytesProcessados"], df["L_Barra_Acumulado"], color="green", linewidth=2,             label="L_Barra Acumulado")

# Marca os resets como linhas verticais vermelhas
for x in resets["BytesProcessados"]:
    ax1.axvline(x=x, color="red", alpha=0.5, linewidth=1)

ax1.set_ylabel("Bits por Símbolo")
ax1.set_title(f"Diagnóstico de Resets — {arquivo_csv.split('/')[-1]}")
ax1.legend(fontsize=9)
ax1.grid(True, linestyle=":", alpha=0.6)

# --- GRÁFICO 2: BitsNaJanela bruto (revela ruído de medição) ---
ax2.plot(df["BytesProcessados"], df["BitsNaJanela"], color="purple", linewidth=1, label="Bits na Janela (bruto)")
for x in resets["BytesProcessados"]:
    ax2.axvline(x=x, color="red", alpha=0.5, linewidth=1)

ax2.set_ylabel("Bits na Janela")
ax2.set_xlabel("Bytes Processados")
ax2.legend(fontsize=9)
ax2.grid(True, linestyle=":", alpha=0.6)

plt.tight_layout()
plt.savefig(arquivo_csv + "_diagnostico.png", dpi=300)
plt.show()

# --- DIAGNÓSTICO NO TERMINAL ---
print(f"\nTotal de resets: {int(df['ResetCount'].max())}")
print(f"\nJanelas que dispararam reset:")
print(resets[["BytesProcessados", "L_Barra_Janela", "SmoothedBPS", "ThresholdBPS"]].to_string(index=False))
