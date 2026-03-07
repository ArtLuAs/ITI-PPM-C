import pandas as pd
import matplotlib.pyplot as plt
import sys

# Coloque aqui o caminho do CSV gerado pelo C++
arquivo_csv = "saidas/dickens_comprimido.bin_grafico.csv"

try:
    df = pd.read_csv(arquivo_csv)
    
    # Remove qualquer linha vazia ou com erro (NaN) que possa ter vindo do C++
    df = df.dropna()
    
except FileNotFoundError:
    print(f"Erro: Arquivo {arquivo_csv} nao encontrado. Rode o C++ primeiro.")
    sys.exit()

# --- DIAGNÓSTICO: Mostra as 5 primeiras linhas do CSV na tela ---
print("--- DADOS LIDOS DO CSV ---")
print(df.head())
print("--------------------------")

if df.empty:
    print("Erro: O arquivo CSV está vazio ou todas as linhas continham erros.")
    sys.exit()

# Pega a entropia da primeira linha válida com segurança (.iloc[0])
entropia_val = df['Entropia_Ordem0'].iloc[0]


plt.figure(figsize=(10, 6))

# Plot da Entropia (Linha reta constante)
plt.axhline(y=entropia_val, color='red', linestyle='--', linewidth=2, 
            label=f"Entropia (Ordem-0): {entropia_val:.2f} bits")

# Plot do BPS Acumulado
plt.plot(df['BytesProcessados'], df['L_Barra_Acumulado'], 
         color='blue', linewidth=2, label=r"$\overline{l}$ (Médio Acumulado)")

# Plot do BPS da Janela
plt.plot(df['BytesProcessados'], df['L_Barra_Janela'], 
         color='cyan', alpha=0.4, linewidth=1, label=r"$\overline{l}$ (Janelas de 1000 bytes)")

# Configurações estéticas
plt.title(r'Convergência do Comprimento Médio ($\overline{l}$) vs Entropia ($H$)', fontsize=14)
plt.xlabel('Bytes Processados do Arquivo', fontsize=12)
plt.ylabel('Bits Por Símbolo (BPS)', fontsize=12)
plt.legend(loc='upper right', fontsize=11)
plt.grid(True, linestyle=':', alpha=0.7)

ymin = min(df['L_Barra_Acumulado'].min(), df['L_Barra_Janela'].min()) * 0.8
ymax = max(df['L_Barra_Acumulado'].max(), entropia_val) * 1.2
plt.ylim(ymin, ymax)

plt.tight_layout()

plt.savefig(arquivo_csv + ".png", dpi=300)
plt.show()