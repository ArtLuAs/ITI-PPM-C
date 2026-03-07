#include "../headers/PPM.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <map>
#include <vector>
#include "../headers/ArithmeticCoder.hpp"
#include "../headers/ContextModel.hpp"

using namespace std;

void compressPPM(const string& input, const string& outputFile, int order) {

// --------------------------------------------------------
    // CÁLCULO DA ENTROPIA DE ORDEM-0 (Baseline de Shannon)
    // --------------------------------------------------------
   map<unsigned char, int> freqMap;
    for (unsigned char c : input) {
        freqMap[c]++;
    }
    
    double entropia = 0.0;
    
    // TRAVA 1: Cast explícito para double para forçar divisão de ponto flutuante
    double tamanhoTotalStr = static_cast<double>(input.length());
    
    // TRAVA 2: Impede divisão por zero caso o arquivo lido esteja completamente vazio
    if (tamanhoTotalStr > 0.0) { 
        for (auto const& [c, count] : freqMap) {
            
            // TRAVA 3: Se a contagem for 0, ignoramos, pois na fórmula de Shannon 0*log(0) = 0
            if (count > 0) { 
                double probabilidade = static_cast<double>(count) / tamanhoTotalStr;
                entropia -= probabilidade * log2(probabilidade);
            }
        }
    }
    
    cout << "Entropia de Ordem-0 do arquivo: " << entropia << " bits/char" << endl;
    // --------------------------------------------------------
    ofstream outStream(outputFile, ios::binary);
    BitOutputStream bitOut(outStream);
    ArithmeticEncoder encoder(32, bitOut);
    
    ContextModel model(order);
    vector<bool> isExcluded(259, false); // 258 
    // --- ABRINDO O ARQUIVO CSV PARA O GRÁFICO ---
    string csvName = outputFile + "_grafico.csv";
    ofstream csvOut(csvName);
    csvOut << "BytesProcessados,L_Barra_Janela,L_Barra_Acumulado,Entropia_Ordem0\n";
    // --------------------------------------------

    auto encodeSymbol = [&](uint32_t symbol) {
        vector<TrieNode*> activeNodes = model.getActiveContextNodes();

        for (TrieNode* node : activeNodes) {
            for (uint32_t i = 0; i < 259; ++i) { // laço até 259? ou 258
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

        // Não atualiza o modelo se for o EOF (257) nem reset (258)
        if (symbol != 257 && symbol != 258) model.updateAndShift(symbol);
    };

    cout << "Comprimindo com monitoramento de Janela e Exportacao CSV..." << endl;
    
    // --- VARIÁVEIS DE MONITORAMENTO DA JANELA ---
    int j = 5000; // 1. AUMENTADO: 5000 é mais seguro contra o ruído do buffer
    int symbolsInWindow = 0;
    long previousBitSize = 0;
    
    double smoothedBPS = 0.0; // Usaremos Média Móvel em vez de valor absoluto
    double thresholdPercent = 0.20; // 20% de tolerância para flutuações naturais
    double safetyFloorBPS = 5.0; // 2. PISO: Só reseta se o BPS estiver pior que 5.0 bits/símbolo

    long totalSymbolsProcessed = 0;

    for (unsigned char c : input) {
        encodeSymbol(c);
        symbolsInWindow++;
        totalSymbolsProcessed++;

        if (symbolsInWindow == j) {
            outStream.flush(); 
            long currentBitSize = static_cast<long>(outStream.tellp()) * 8;
            long bitsInWindow = currentBitSize - previousBitSize;
            
            double currentBPS = static_cast<double>(bitsInWindow) / j;
            double accumulatedBPS = static_cast<double>(currentBitSize) / totalSymbolsProcessed;
            
            // Grava os pontos no CSV
            csvOut << totalSymbolsProcessed << "," 
                   << currentBPS << "," 
                   << accumulatedBPS << "\n"; // (Tirei a entropia pois não estava declarada no seu snippet)

            // Inicializa o suavizador na primeira janela
            if (smoothedBPS == 0.0) {
                smoothedBPS = currentBPS;
            }

            // 3. A NOVA AVALIAÇÃO DE PERFORMANCE
            // Só reseta se: Piorou > 20% em relação à média E o BPS atual está realmente ruim (> 5.0)
            if (currentBPS > smoothedBPS * (1.0 + thresholdPercent) && currentBPS > safetyFloorBPS) {
                cout << "[!] Mudanca drastica detectada (Media: " << fixed << setprecision(3) 
                     << smoothedBPS << " -> Atual: " << currentBPS << " BPS). Resetando o modelo!" << endl;
                
                encodeSymbol(258); 
                model.reset(); 
                
                smoothedBPS = 0.0; // Zera para reaprender do zero
            } else {
                // Média Móvel Exponencial: 70% do peso no passado, 30% na janela atual.
                // Isso amortece os picos falsos do buffer!
                smoothedBPS = (smoothedBPS * 0.7) + (currentBPS * 0.3);
            }
            
            previousBitSize = currentBitSize; // Aqui usamos o currentBitSize diretamente, é mais seguro que chamar tellp() de novo
            symbolsInWindow = 0;
        }
    }
    
    encodeSymbol(257); // Emite o EOF verdadeiro (257) no fim do texto

    encoder.finish();
    bitOut.finish();
    outStream.close();
    csvOut.close(); 
        cout << "Compressao concluida! Dados do grafico salvos em: " << csvName << endl;
}