#include <iostream>
#include <fstream>
#include <string>
#include "headers/PPM.hpp"

using namespace std;

int main() {
    // string textoOriginal = "abracadabra, o rato roeu a roupa do rei de roma! aaaaa bbbbb ccccc";
    string arquivoComprimido = "teste_ppm.bin";
    string arquivoDescomprimido = "saida_descomprimida.txt";
    int ordemK = 10; 

    // Prepara a entrada
    // ofstream outOriginal("entrada.txt");
    // outOriginal << textoOriginal;
    // outOriginal.close();

    ifstream inOriginal("documentos/memoriaspostumas.txt");
    string inputStr((istreambuf_iterator<char>(inOriginal)), istreambuf_iterator<char>());
    inOriginal.close();

    // 1. Comprime
    compressPPM(inputStr, arquivoComprimido, ordemK);

    // 2. Descomprime
    decompressPPM(arquivoComprimido, arquivoDescomprimido, ordemK);

    // 3. Validação
    ifstream inDescomprimido(arquivoDescomprimido);
    string resultStr((istreambuf_iterator<char>(inDescomprimido)), istreambuf_iterator<char>());
    inDescomprimido.close();

    if (inputStr == resultStr) {
        cout << "\nSUCESSO: Compressao e descompressao validadas sem perdas!" << endl;
    } else {
        cout << "\nERRO: Os dados recuperados diferem da entrada original." << endl;
    }

    return 0;
}