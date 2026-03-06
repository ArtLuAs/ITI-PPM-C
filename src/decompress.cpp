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
    vector<bool> isExcluded(258, false); // até 258

    cout << "Descomprimindo..." << endl;

    while (true) {
        uint32_t decodedSymbol = 0;
        vector<TrieNode*> activeNodes = model.getActiveContextNodes();

        for (TrieNode* node : activeNodes) {
            for (uint32_t i = 0; i < 258; ++i) { // laço ate 258
                if (isExcluded[i]) node->freqTable->excludeSymbol(i);
            }

            uint32_t uniqueSymbols = node->activeSymbols.size();
            uint32_t escapeWeight = (uniqueSymbols > 0) ? uniqueSymbols : 1; 
            node->freqTable->set(256, escapeWeight);

            decodedSymbol = decoder.read(*(node->freqTable));
            node->freqTable->restoreExcludedSymbols();

            if (decodedSymbol == 256) { // Escape
                for (uint32_t activeSym : node->activeSymbols) {
                    isExcluded[activeSym] = true;
                }
            } else {
                break; // Achou o símbolo real
            }
        }

        if (decodedSymbol == 257) break; // Chegou no EOF real da Ordem-0

        outStream.put(static_cast<char>(decodedSymbol));
        fill(isExcluded.begin(), isExcluded.end(), false);
        model.updateAndShift(decodedSymbol);
    }

    outStream.close();
    inStream.close();
    cout << "Descompressao concluida!" << endl;
}