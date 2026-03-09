#!/bin/bash

echo "========================================="
echo "PASSO 1: Compilando o projeto C++..."
echo "========================================="
g++ src/*.cpp *.cpp -o ppm_compressao -O3 -std=c++17

# Verifica se a compilação deu erro
if [ $? -ne 0 ]; then
    echo "❌ Erro na compilação. Abortando o teste."
    exit 1
fi
echo "✅ Compilação concluída com sucesso!"
echo ""

echo "========================================="
echo "PASSO 2: Executando a Compressão PPM..."
echo "========================================="
# Roda o seu programa com ordem K=4 e salva o CSV
./ppm_compressao 4 benchmark_silesia.csv
echo ""

echo "========================================="
echo "PASSO 3: (Comando cmp)"
echo "========================================="

# A mesma lista de arquivos que você tem no seu main.cpp
arquivos=("mr" "ooffice" "mozilla" "dickens" "xml" "osdb" "x-ray" "reymont" "nci" "samba" "webster" "sao" "silesia.tar")
# arquivos=("xml")

for nome in "${arquivos[@]}"; do
    original="silesia/$nome"
    
    # Lógica para extrair o nome e a extensão (igual o seu C++ faz)
    filename=$(basename -- "$original")
    extension="${filename##*.}"
    stem="${filename%.*}"
    
    # Monta o nome exato do arquivo que o seu C++ gerou na pasta saidas/
    if [ "$filename" = "$extension" ]; then
        # Arquivo sem extensão (ex: dickens)
        descomprimido="saidas/${stem}_descomprimido"
    else
        # Arquivo com extensão (ex: silesia.tar)
        descomprimido="saidas/${stem}_descomprimido.${extension}"
    fi

    # Se o arquivo descomprimido existir, fazemos a comparação!
    if [ -f "$descomprimido" ]; then
        # O parâmetro '-s' (silent) faz o cmp não imprimir texto extra, 
        # ele apenas retorna VERDADEIRO (0) se for igual, ou FALSO (1) se diferir.
        if cmp -s "$original" "$descomprimido"; then
            echo "✅ $filename: ARQUIVOS IDENTICOS!" 
        else
            echo "❌ $filename: ATENÇÃO! Os arquivos diferem byte a byte."
        fi
    else
        echo "⚠️  $filename: Arquivo descomprimido não foi encontrado na pasta."
    fi
done

echo "========================================="
echo "🏁 Teste finalizado!"