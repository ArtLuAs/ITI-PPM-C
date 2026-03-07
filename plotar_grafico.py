import sys

import matplotlib.pyplot as plt
import pandas as pd

# Coloque o caminho do seu arquivo CSV aqui
arquivo_csv = "saidas/dickens_comprimido.bin_grafico.csv"

try:
    df = pd.read_csv(arquivo_csv)
except FileNotFoundError:
    print(f"Erro: Arquivo {arquivo_csv} nao encontrado. Rode o C++ primeiro.")
    sys.exit()

plt.figure(figsize=(11, 7))

# 1. LINHAS RETAS (As Entropias teóricas que NÃO mudam)
plt.axhline(y=df['Entropia_Ordem0'][0], color='red', linestyle='--', linewidth=2, 
            label=f"Entropia (Ordem-0): {df['Entropia_Ordem0'][0]:.2f} bits")

if 'Entropia_OrdemK' in df.columns and 'Valor_K' in df.columns:
    ordem_k = int(df['Valor_K'][0])
    entropia_k = df['Entropia_OrdemK'][0]
    plt.axhline(y=entropia_k, color='green', linestyle='-.', linewidth=2, 
                label=f"Entropia (Ordem-{ordem_k}): {entropia_k:.2f} bits")

# 2. CURVAS DESCENDENTES (A performance adaptativa do seu compressor)

# L_Barra Teórico Adaptativo (A soma dos -log2 das probabilidades)
if 'L_Barra_Teorico' in df.columns:
    plt.plot(df['BytesProcessados'], df['L_Barra_Teorico'], 
             color='purple', linewidth=2, linestyle=':', 
             label="$\overline{l}$ (Teórico Adaptativo)")

# L_Barra Real Acumulado (O que o seu codificador aritmético realmente gerou)
plt.plot(df['BytesProcessados'], df['L_Barra_Acumulado'], 
         color='blue', linewidth=2, label="$\overline{l}$ (Real do Codificador)")

# L_Barra da Janela (Mostra os picos que ativam o seu Reset)
plt.plot(df['BytesProcessados'], df['L_Barra_Janela'], 
         color='cyan', alpha=0.3, linewidth=1, label="$\overline{l}$ (Janelas 1000B)")

# Configurações estéticas
plt.title('Performance do PPM: Comprimento Médio vs Entropias Teóricas', fontsize=15)
plt.xlabel('Bytes Processados', fontsize=12)
plt.ylabel('Bits Por Símbolo (BPS)', fontsize=12)
plt.legend(loc='upper right', fontsize=11)
plt.grid(True, linestyle=':', alpha=0.7)

# Ajuste automático do eixo Y (Começa do zero para vermos bem a ordem-K)
ymin = 0.0
ymax = max(df['L_Barra_Acumulado'].max(), df['Entropia_Ordem0'][0]) * 1.2
plt.ylim(ymin, ymax)

plt.tight_layout()

# Salva a imagem no disco e exibe na tela
plt.savefig(arquivo_csv + ".png", dpi=300)
plt.show()