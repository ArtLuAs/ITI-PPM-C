#include "headers/PPM.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
        cout << unitbuf;

        vector<string> listaArquivos = {
            "silesia/mr",
            "silesia/ooffice",
            "silesia/mozilla",
            "silesia/dickens",
            "silesia/xml",
            "silesia/osdb",
            "silesia/x-ray",
            "silesia/reymont",
            "silesia/nci",
            "silesia/samba",
            "silesia/webster",
            "silesia/sao",
            "silesia/xml",
            "silesia/silesia.tar"};

        int ordemK = (argc > 1) ? stoi(argv[1]) : 4;
        string benchmarkCsv = (argc > 2) ? argv[2] : "";

        ofstream benchOut;
        if (!benchmarkCsv.empty()) {
                benchOut.open(benchmarkCsv, ios::app);
                if (!benchOut.is_open()) {
                        cerr << "ERRO: nao foi possivel abrir o CSV de benchmark: " << benchmarkCsv << endl;
                        return 1;
                }
        }

        uintmax_t totalOriginal = 0;
        uintmax_t totalComprimido = 0;
        double totalTempoComp = 0.0;
        double totalTempoDecomp = 0.0;
        bool totalOk = true;

        for (const auto& nomeArquivo : listaArquivos) {
                cout << "\n=======================================================\n";
                cout << "Processando arquivo: " << nomeArquivo << endl;

                fs::path caminho(nomeArquivo);
                string stem = caminho.stem().string();
                string extensao = caminho.extension().string();

                string arquivoComprimido = "saidas/" + stem + "_comprimido.bin";
                string arquivoDescomprimido = "saidas/" + stem + "_descomprimido" + extensao;

                ifstream inOriginal(nomeArquivo, ios::binary);
                if (!inOriginal.is_open()) {
                        cerr << "ERRO: Nao foi possivel abrir o arquivo original: " << nomeArquivo << endl;
                        totalOk = false;
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

                ifstream inDescomprimido(arquivoDescomprimido, ios::binary);
                if (!inDescomprimido.is_open()) {
                        cerr << "ERRO: Nao foi possivel abrir o arquivo gerado: " << arquivoDescomprimido << endl;
                        totalOk = false;
                        continue;
                }

                string resultStr((istreambuf_iterator<char>(inDescomprimido)), istreambuf_iterator<char>());
                inDescomprimido.close();

                bool ok = (inputStr == resultStr);
                if (ok) {
                        cout << "SUCESSO: Compressao e descompressao validadas sem perdas!" << endl;
                } else {
                        cout << "ERRO: Os dados recuperados diferem da entrada original." << endl;
                        totalOk = false;
                }

                chrono::duration<double> duration1 = end_compress - start_compress;
                chrono::duration<double> duration2 = end_decompress - start_decompress;

                double tempoComp = duration1.count();
                double tempoDecomp = duration2.count();

                cout << "DURACAO COMPRESSAO: " << tempoComp << endl;
                cout << "DURACAO DESCOMPRESSAO: " << tempoDecomp << endl;
                cout << "DURACAO TOTAL: " << (tempoComp + tempoDecomp) << endl;

                try {
                        auto tamanhoOriginal = fs::file_size(nomeArquivo);
                        auto tamanhoComprimido = fs::file_size(arquivoComprimido);

                        double bps = (static_cast<double>(tamanhoComprimido) * 8.0) / tamanhoOriginal;
                        double razaoCompressao = (static_cast<double>(tamanhoComprimido) / tamanhoOriginal) * 100.0;

                        cout << "\n--- METRICAS DE COMPRESSAO ---" << endl;
                        cout << "Tamanho Original:    " << tamanhoOriginal << " bytes" << endl;
                        cout << "Tamanho Comprimido:  " << tamanhoComprimido << " bytes" << endl;
                        cout << "BPC (Bits per Char): " << fixed << setprecision(3) << bps << " bits/char" << endl;
                        cout << "Razao de Compressao: " << fixed << setprecision(2) << razaoCompressao << "% do tamanho original" << endl;

                        if (tamanhoComprimido >= tamanhoOriginal) {
                                cout << "[!] Sem ganho de espaco. O arquivo cresceu ou empatou." << endl;
                        } else {
                                cout << "[+] Reducao efetiva de espaco: " << fixed << setprecision(2)
                                     << (100.0 - razaoCompressao) << "%" << endl;
                        }

                        totalOriginal += tamanhoOriginal;
                        totalComprimido += tamanhoComprimido;
                        totalTempoComp += tempoComp;
                        totalTempoDecomp += tempoDecomp;

                        if (benchOut.is_open()) {
                                benchOut << "PPM-C,"
                                         << ordemK << ","
                                         << stem << ","
                                         << tamanhoOriginal << ","
                                         << tamanhoComprimido << ","
                                         << fixed << setprecision(6) << bps << ","
                                         << tempoComp << ","
                                         << tempoDecomp << ","
                                         << (ok ? "OK" : "FAIL")
                                         << "\n";
                        }

                } catch (const fs::filesystem_error& e) {
                        cerr << "Erro ao calcular metricas de tamanho: " << e.what() << endl;
                        totalOk = false;
                }

                cout << "=======================================================\n"
                     << endl;
        }

        if (benchOut.is_open() && totalOriginal > 0) {
                double totalBps = (static_cast<double>(totalComprimido) * 8.0) / totalOriginal;
                benchOut << "PPM-C,"
                         << ordemK << ","
                         << "TOTAL,"
                         << totalOriginal << ","
                         << totalComprimido << ","
                         << fixed << setprecision(6) << totalBps << ","
                         << totalTempoComp << ","
                         << totalTempoDecomp << ","
                         << (totalOk ? "OK" : "FAIL")
                         << "\n";
        }

        return 0;
}
