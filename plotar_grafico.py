import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import sys

arquivo_csv = "saidas/dickens_comprimido.bin_grafico.csv"

try:
    df = pd.read_csv(arquivo_csv)
except FileNotFoundError:
    print(f"Erro: Arquivo {arquivo_csv} nao encontrado. Rode o C++ primeiro.")
    sys.exit()

plt.figure(figsize=(12, 8))

h_cols = [col for col in df.columns if col.startswith('H_')]

cmap = cm.get_cmap('viridis', len(h_cols))

for i, col in enumerate(h_cols):
    ordem = col.split('_')[1]
    valor_entropia = df[col][0]
    
    if ordem == '0':
        plt.axhline(y=valor_entropia, color='red', linestyle='--', linewidth=2.5, 
                    label=f"$H_{{{ordem}}}$ (Lim. de Shannon): {valor_entropia:.2f} bits")
    else:
        plt.axhline(y=valor_entropia, color=cmap(i), linestyle='-.', linewidth=1.5, alpha=0.8,
                    label=f"$H_{{{ordem}}}$: {valor_entropia:.2f} bits")


if 'L_Barra_Teorico' in df.columns:
    plt.plot(df['BytesProcessados'], df['L_Barra_Teorico'], 
             color='purple', linewidth=2.5, linestyle=':', 
             label="$\overline{l}$ (Teórico Adaptativo)")


plt.plot(df['BytesProcessados'], df['L_Barra_Acumulado'], 
         color='blue', linewidth=2.5, label="$\overline{l}$ (Real do Codificador)")


plt.plot(df['BytesProcessados'], df['L_Barra_Janela'], 
         color='cyan', alpha=0.3, linewidth=1, label="$\overline{l}$ (Janelas Buffer)")

plt.title('Degradação da Entropia Empírica e Performance Adaptativa do PPM', fontsize=16)
plt.xlabel('Bytes Processados (Tamanho do Arquivo)', fontsize=12)
plt.ylabel('Bits Por Símbolo (BPS)', fontsize=12)

plt.legend(loc='center left', bbox_to_anchor=(1, 0.5), fontsize=10)
plt.grid(True, linestyle=':', alpha=0.7)

ymin = 0.0
ymax = df['H_0'][0] * 1.2
plt.ylim(ymin, ymax)

plt.tight_layout()

plt.savefig(arquivo_csv + ".png", dpi=300, bbox_inches='tight')
plt.show()