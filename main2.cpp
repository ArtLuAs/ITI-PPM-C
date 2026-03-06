#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem> // Adicione esta biblioteca
#include "headers/PPM.hpp"

using namespace std;

int main() {
    vector<string> listaArquivos = {
        // "silesia/mr", "silesia/ooffice", "silesia/mozilla", 
        "silesia/dickens"
        // , 
        // "silesia/osdb", "silesia/x-ray", "silesia/reymont", "silesia/nci", "silesia/samba", "silesia/webster", "silesia/sao", "silesia/xml"
        // Você pode colocar qualquer formato aqui agora, ex: "documentos/foto.png"
    };

    int ordemK = 8; 

    for (const auto& nomeArquivo : listaArquivos) {
        cout << "\n=======================================================" << endl;
        cout << "Processando arquivo: " << nomeArquivo << endl;

        // 1. Extrai o nome do arquivo sem extensão e a extensão original
        std::filesystem::path caminho(nomeArquivo);
        string stem = caminho.stem().string();       // ex: "memoriaspostumas"
        string extensao = caminho.extension().string(); // ex: ".txt", ".png", etc.

        // 2. Monta o caminho correto na pasta de saídas
        string arquivoComprimido = "saidas/" + stem + "_comprimido.bin";
        string arquivoDescomprimido = "saidas/" + stem + "_descomprimido" + extensao;

        // IMPORTANTE: Abrir em modo binário (ios::binary) para aceitar qualquer formato
        ifstream inOriginal(nomeArquivo, ios::binary);
        
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

        // Também abrir em modo binário na verificação
        ifstream inDescomprimido(arquivoDescomprimido, ios::binary);
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