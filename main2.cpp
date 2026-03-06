#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem> // Adicione esta biblioteca
#include "headers/PPM.hpp"

using namespace std;

int main() {
    vector<string> listaArquivos = {
        "documentos/memoriaspostumas.txt",
        "documentos/entrada.txt"
    };

    int ordemK = 10; 

    for (const auto& nomeArquivo : listaArquivos) {
        cout << "\n=======================================================" << endl;
        cout << "Processando arquivo: " << nomeArquivo << endl;

        // 1. Extrai apenas o nome do arquivo (ex: "memoriaspostumas.txt")
        string nomeBase = std::filesystem::path(nomeArquivo).filename().string();

        // 2. Monta o caminho correto na pasta de saídas
        string arquivoComprimido = "saidas/" + nomeBase + "_comprimido.bin";
        string arquivoDescomprimido = "saidas/" + nomeBase + "_descomprimido.txt";

        // ... o resto do seu código continua exatamente igual ...
        ifstream inOriginal(nomeArquivo);
        
        if (!inOriginal.is_open()) {
            cerr << "ERRO: Nao foi possivel abrir o arquivo original: " << nomeArquivo << endl;
            continue; 
        }

        string inputStr((istreambuf_iterator<char>(inOriginal)), istreambuf_iterator<char>());
        inOriginal.close();

        cout << "-> Comprimindo para: " << arquivoComprimido << endl;
        compressPPM(inputStr, arquivoComprimido, ordemK);

        cout << "-> Descomprimindo para: " << arquivoDescomprimido << endl;
        decompressPPM(arquivoComprimido, arquivoDescomprimido, ordemK);

        ifstream inDescomprimido(arquivoDescomprimido);
        if (!inDescomprimido.is_open()) {
            cerr << "ERRO: Nao foi possivel abrir o arquivo gerado: " << arquivoDescomprimido << endl;
            continue;
        }

        string resultStr((istreambuf_iterator<char>(inDescomprimido)), istreambuf_iterator<char>());
        inDescomprimido.close();

        if (inputStr == resultStr) {
            cout << "SUCESSO: Compressao e descompressao validadas sem perdas!" << endl;
        } else {
            cout << "ERRO: Os dados recuperados diferem da entrada original." << endl;
        }
    }

    return 0;
}