#include <iostream>
#include <fstream>
#include <vector>
#include "ArithmeticCoder.hpp"
#include "tabelaFrequencia.hpp" // Adicione o header da tabela de frequências

using namespace std;

int main() {
    // ==========================================
    // PARTE 1: COMPRESSÃO
    // ==========================================
    ofstream outStream("teste_comprimido.bin", ios::binary);
    BitOutputStream bitOut(outStream);
    ArithmeticEncoder encoder(32, bitOut);

    string texto = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut quam nulla, fringilla sed efficitur at, consectetur eu orci. Praesent consectetur mauris ac tempus ullamcorper. Sed tortor purus, tempor eu hendrerit quis, consequat vel mi. Cras finibus tincidunt tincidunt. Donec dapibus enim at orci rutrum sodales. Pellentesque non metus tortor. Suspendisse potenti.";

    // Cria vetor zerado (0 a 255 para chars, 256 para EOF)
    vector<uint32_t> frequencias(257, 0);

    // Conta todas as ocorrências (modelo semi-adaptativo)
    for (unsigned char c : texto) {
        frequencias[c]++;
    }

    // EOF precisa ter pelo menos 1 de frequência para existir no intervalo
    frequencias[256] = 1;

    // Criamos a tabela
    SimpleFrequencyTable freqTable(frequencias);

    // Codificamos caractere por caractere
    for (unsigned char c : texto) {
        encoder.write(freqTable, c);
    }

    // Escrevemos o símbolo de EOF para marcar o fim
    encoder.write(freqTable, 256);

    // Finaliza os fluxos de bits e fecha o arquivo
    encoder.finish();
    bitOut.finish();
    outStream.close();

    cout << "Compressao de teste concluida com sucesso!" << endl;


    // ==========================================
    // PARTE 2: DESCOMPRESSÃO
    // ==========================================
    ifstream inStream("teste_comprimido.bin", ios::binary);
    BitInputStream bitIn(inStream);
    
    // Inicia o decodificador com os mesmos 32 bits
    ArithmeticDecoder decoder(32, bitIn);

    string textoDecodificado = "";

    // Loop infinito até encontrarmos o EOF
    while (true) {
        // O read() faz a mágica: acha o símbolo correspondente aos bits lidos
        // usando a MESMA tabela de frequências da compressão.
        uint32_t symbol = decoder.read(freqTable);

        // Se o símbolo lido for o nosso EOF (256), paramos o processo
        if (symbol == 256) {
            break;
        }

        // Caso contrário, convertemos de volta para char e adicionamos à string
        textoDecodificado += static_cast<char>(symbol);
    }

    inStream.close();

    cout << "\nTexto decodificado: " << textoDecodificado << endl;
    
    if (texto == textoDecodificado) {
        cout << "Sucesso: O texto original e o decodificado sao identicos!" << endl;
    } else {
        cout << "Erro: Houve perda de dados." << endl;
    }

    return 0;
}