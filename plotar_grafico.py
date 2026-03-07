import sys

import matplotlib.pyplot as plt
import pandas as pd

# Coloque aqui o caminho do CSV gerado pelo C++
arquivo_csv = "saidas/dickens_comprimido.bin_grafico.csv"

try:
    df = pd.read_csv(arquivo_csv)
except FileNotFoundError:
    print(f"Erro: Arquivo {arquivo_csv} nao encontrado. Rode o C++ primeiro.")
    sys.exit()

plt.figure(figsize=(10, 6))

# Plot da Entropia (Linha reta constante)
plt.axhline(y=df['Entropia_Ordem0'][0], color='red', linestyle='--', linewidth=2, 
            label=f"Entropia (Ordem-0): {df['Entropia_Ordem0'][0]:.2f} bits")

# Plot do BPS Acumulado (Tendência geral convergindo)
plt.plot(df['BytesProcessados'], df['L_Barra_Acumulado'], 
         color='blue', linewidth=2, label="$\overline{l}$ (Médio Acumulado)")

# Plot do BPS da Janela (Mostra os picos locais de forma ruidosa)
plt.plot(df['BytesProcessados'], df['L_Barra_Janela'], 
         color='cyan', alpha=0.4, linewidth=1, label="$\overline{l}$ (Janelas de 1000 bytes)")

# Configurações estéticas do gráfico
plt.title('Convergência do Comprimento Médio ($\overline{l}$) vs Entropia ($H$)', fontsize=14)
plt.xlabel('Bytes Processados do Arquivo', fontsize=12)
plt.ylabel('Bits Por Símbolo (BPS)', fontsize=12)
plt.legend(loc='upper right', fontsize=11)
plt.grid(True, linestyle=':', alpha=0.7)

# Ajuste automático do eixo Y para focar onde as linhas estão
ymin = min(df['L_Barra_Acumulado'].min(), df['L_Barra_Janela'].min()) * 0.8
ymax = max(df['L_Barra_Acumulado'].max(), df['Entropia_Ordem0'][0]) * 1.2
plt.ylim(ymin, ymax)

plt.tight_layout()

# Salva a imagem no disco (opcional) e exibe na tela
plt.savefig(arquivo_csv + ".png", dpi=300)
plt.show()