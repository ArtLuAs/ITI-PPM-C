#include "../headers/PPM.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include "../headers/ArithmeticCoder.hpp"
#include "../headers/ContextModel.hpp"

using namespace std;

void compressPPM(const string& input, const string& outputFile, int order) {
    ofstream outStream(outputFile, ios::binary);
    BitOutputStream bitOut(outStream);
    ArithmeticEncoder encoder(32, bitOut);
    
    ContextModel model(order);
    vector<bool> isExcluded(258, false); // 258 

    auto encodeSymbol = [&](uint32_t symbol) {
        vector<TrieNode*> activeNodes = model.getActiveContextNodes();

        for (TrieNode* node : activeNodes) {
            for (uint32_t i = 0; i < 258; ++i) { // laço até 258? ou 257
                if (isExcluded[i]) node->freqTable->excludeSymbol(i);
            }

            // O peso do Escape é igual à quantidade de símbolos únicos neste contexto
            uint32_t uniqueSymbols = node->activeSymbols.size();
            // Garante peso mínimo de 1 para evitar probabilidade zero caso o nó seja novo
            uint32_t escapeWeight = (uniqueSymbols > 0) ? uniqueSymbols : 1; 
            node->freqTable->set(256, escapeWeight);
            
            if (node->freqTable->get(symbol) > 0) {
                encoder.write(*(node->freqTable), symbol);
                node->freqTable->restoreExcludedSymbols();
                break; 
            } else {
                encoder.write(*(node->freqTable), 256); // Escape
                for (uint32_t activeSym : node->activeSymbols) {
                    isExcluded[activeSym] = true;
                }
                node->freqTable->restoreExcludedSymbols();
            }
        }

        fill(isExcluded.begin(), isExcluded.end(), false);

        // Não atualiza o modelo se for o EOF (257)
        if (symbol != 257) model.updateAndShift(symbol);
    };

    cout << "Comprimindo..." << endl;
    for (unsigned char c : input) {
        encodeSymbol(c);
    }
    encodeSymbol(257); // EOF verdadeiro (257)

    encoder.finish();
    bitOut.finish();
    outStream.close();
    
    cout << "Compressao concluida!" << endl;
}