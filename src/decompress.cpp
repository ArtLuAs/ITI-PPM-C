#include "../headers/PPM.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include "../headers/ArithmeticCoder.hpp"
#include "../headers/ContextModel.hpp"

using namespace std;

void decompressPPM(const string& inputFile, const string& outputFile, int order) {
    ifstream inStream(inputFile, ios::binary);
    if (!inStream.is_open()) {
        cerr << "Erro ao abrir o arquivo comprimido!" << endl;
        return;
    }

    BitInputStream bitIn(inStream);
    ArithmeticDecoder decoder(32, bitIn);
    ofstream outStream(outputFile, ios::binary);

    ContextModel model(order);
    
    // O vetor de exclusão vai de 0 a 258
    vector<bool> isExcluded(259, false);

    cout << "Descomprimindo..." << endl;

    while (true) {
        uint32_t decodedSymbol = 0;
        vector<TrieNode*> activeNodes = model.getActiveContextNodes();

        for (TrieNode* node : activeNodes) {
            
            // Exclusões vindas dos escapes anteriores
            for (uint32_t i = 0; i < 259; ++i) {
                if (isExcluded[i]) node->freqTable->excludeSymbol(i);
            }

            // Cálculo do Peso do Escape
            uint32_t uniqueSymbols = node->activeSymbols.size();
            uint32_t escapeWeight = (uniqueSymbols > 0) ? uniqueSymbols : 1; 
            node->freqTable->set(256, escapeWeight);

            // Símbolo do arquivo comprimido
            decodedSymbol = decoder.read(*(node->freqTable));

            // Restaura a tabela imediatamente após a leitura
            node->freqTable->restoreExcludedSymbols();

            // Escape 
            if (decodedSymbol == 256) {
                for (uint32_t activeSym : node->activeSymbols) {
                    isExcluded[activeSym] = true;
                }
            } else {
                // Símbolo real (Char, EOF ou RESET)
                break; 
            }
        }

        // piora da janela
        if (decodedSymbol == 258) {
            cout << "[!] Comando de RESET recebido. Reiniciando modelo..." << endl;
            
            model.reset();
            
            // Limpa a exclusão e pula para ler a PRÓXIMA letra 
            fill(isExcluded.begin(), isExcluded.end(), false);
            continue; 
        }

        // Fim de arquivo verdadeiro encontrado na Ordem-0
        if (decodedSymbol == 257) {
            break; 
        }

        // Se não foi controle (nem 258 nem 257), escreve o caractere normal
        outStream.put(static_cast<char>(decodedSymbol));
        
        // Limpa a máscara de exclusão para o próximo caractere
        fill(isExcluded.begin(), isExcluded.end(), false);
        
        // Atualiza a árvore da Trie e a janela deslizante
        model.updateAndShift(decodedSymbol);
    }

    outStream.close();
    inStream.close();
    cout << "Descompressao concluida!" << endl;
}