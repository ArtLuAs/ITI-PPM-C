#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem> // Adicione esta biblioteca
#include "headers/PPM.hpp"

using namespace std;

// mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

int main() {
    vector<string> listaArquivos = {
        // "silesia/mr", "silesia/ooffice", "silesia/mozilla", 
        "silesia/dickens", "silesia/xml"
        // , 
        // "silesia/osdb", "silesia/x-ray", "silesia/reymont", "silesia/nci", "silesia/samba", "silesia/webster", "silesia/sao", "silesia/xml"
        // Você pode colocar qualquer formato aqui agora, ex: "documentos/foto.png"
    };

    int ordemK = 4; 

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
        
        auto start_compress = chrono::high_resolution_clock::now();

        cout << "-> Comprimindo para: " << arquivoComprimido << endl;
        compressPPM(inputStr, arquivoComprimido, ordemK);

        auto end_compress = chrono::high_resolution_clock::now();

        auto start_decompress = chrono::high_resolution_clock::now();

        cout << "-> Descomprimindo para: " << arquivoDescomprimido << endl;
        decompressPPM(arquivoComprimido, arquivoDescomprimido, ordemK);

        auto end_decompress = chrono::high_resolution_clock::now();

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

        chrono::duration<double> duration1 = end_compress - start_compress;
        chrono::duration<double> duration2 = end_decompress - start_decompress;

        cout << "DURACAO COMPRESSAO: " << (double) (duration1.count()) << endl;
        cout << "DURACAO DESCOMPRESSAO: " << (double) (duration2.count()) << endl;
        cout << "DURACAO TOTAL: " << (double) (duration1.count() + duration2.count()) << endl;

        // ... seus couts de DURACAO ...

        // --- EXPERIMENTOS E AVALIACAO ---
        try {
            // std::filesystem::file_size retorna o tamanho em bytes
            auto tamanhoOriginal = std::filesystem::file_size(nomeArquivo);
            auto tamanhoComprimido = std::filesystem::file_size(arquivoComprimido);

            // BPC (Bits Por Caractere): (Bytes Comprimidos * 8) / Bytes Originais
            double bpc = (static_cast<double>(tamanhoComprimido) * 8.0) / tamanhoOriginal;

            // Razão de Compressão (Porcentagem do tamanho original)
            double razaoCompressao = (static_cast<double>(tamanhoComprimido) / tamanhoOriginal) * 100.0;

            cout << "\n--- METRICAS DE COMPRESSAO ---" << endl;
            cout << "Tamanho Original:    " << tamanhoOriginal << " bytes" << endl;
            cout << "Tamanho Comprimido:  " << tamanhoComprimido << " bytes" << endl;
            
            // Usando setprecision para formatar a saída com 3 casas decimais
            cout << "BPC (Bits per Char): " << fixed << setprecision(3) << bpc << " bits/char" << endl;
            cout << "Razao de Compressao: " << fixed << setprecision(2) << razaoCompressao << "% do tamanho original" << endl;
            
            if (tamanhoComprimido >= tamanhoOriginal) {
                cout << "[!] Sem ganho de espaco. O arquivo cresceu ou empatou." << endl;
            } else {
                cout << "[+] Reducao efetiva de espaco: " << fixed << setprecision(2) << (100.0 - razaoCompressao) << "%" << endl;
            }

        } catch (const std::filesystem::filesystem_error& e) {
            cerr << "Erro ao calcular metricas de tamanho: " << e.what() << endl;
        }
        cout << "=======================================================\n" << endl;
    }

    return 0;
}