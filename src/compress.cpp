#include "../headers/PPM.hpp"
#include <iostream>
#include <unordered_map>
#include <string_view>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <map>
#include <vector>
#include "../headers/ArithmeticCoder.hpp"
#include "../headers/ContextModel.hpp"

using namespace std;

double calcularEntropiaOrdemK(const string& input, int k) {
    if (input.length() <= static_cast<size_t>(k)) return 0.0;

    unordered_map<string_view, int> contextFreq;
    unordered_map<string_view, int> contextCharFreq;

    size_t nContexts = input.length() - k;

    // 1. Conta as frequências de todos os contextos (tamanho k) 
    // e contextos + caractere seguinte (tamanho k+1)
    for (size_t i = 0; i < nContexts; ++i) {
        string_view ctx(input.data() + i, k);
        string_view ctxChar(input.data() + i, k + 1);

        contextFreq[ctx]++;
        contextCharFreq[ctxChar]++;
    }

    double entropiaK = 0.0;
    double nContexts_d = static_cast<double>(nContexts);

    // 2. Aplica a fórmula da Entropia Condicional
    for (const auto& [ctxChar, count_ctxChar] : contextCharFreq) {
        string_view ctx = ctxChar.substr(0, k);
        int count_ctx = contextFreq[ctx];

        double p_cx = static_cast<double>(count_ctxChar) / nContexts_d; // P(Contexto e Char)
        double p_x_given_c = static_cast<double>(count_ctxChar) / static_cast<double>(count_ctx); // P(Char | Contexto)

        entropiaK -= p_cx * log2(p_x_given_c);
    }

    return entropiaK;
}

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
    double entropiaOrdemK = calcularEntropiaOrdemK(input, order);
    cout << "Entropia Empirica de Ordem-" << order << " do arquivo: " << entropiaOrdemK << " bits/char" << endl;
    // --------------------------------------------------------
    
    ofstream outStream(outputFile, ios::binary);
    BitOutputStream bitOut(outStream);
    ArithmeticEncoder encoder(32, bitOut);
    
    ContextModel model(order);
    vector<bool> isExcluded(259, false); // 258 
    
    // --- ABRINDO O ARQUIVO CSV PARA O GRÁFICO ---
    string csvName = outputFile + "_grafico.csv";
    ofstream csvOut(csvName);
    
    // 1. ATUALIZAÇÃO DO CABEÇALHO DO CSV COM AS NOVAS MÉTRICAS
    csvOut << "BytesProcessados,L_Barra_Janela,L_Barra_Acumulado,Entropia_Ordem0,Entropia_OrdemK,Valor_K,L_Barra_Teorico\n";
    // --------------------------------------------

    // 2. VARIÁVEL PARA O L_BARRA TEÓRICO ADAPTATIVO
    double informacaoAdaptativaAcumulada = 0.0;

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
                // --- NOVO: Cálculo da Informação do Símbolo no instante de predição ---
                double p = static_cast<double>(node->freqTable->get(symbol)) / node->freqTable->getTotal();
                informacaoAdaptativaAcumulada += -log2(p);
                // ----------------------------------------------------------------------

                encoder.write(*(node->freqTable), symbol);
                node->freqTable->restoreExcludedSymbols();
                break; 
            } else {
                // --- NOVO: Cálculo da Informação do Escape no instante de predição ---
                double p_escape = static_cast<double>(node->freqTable->get(256)) / node->freqTable->getTotal();
                informacaoAdaptativaAcumulada += -log2(p_escape);
                // ---------------------------------------------------------------------

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
            
            // 3. NOVO: L_Barra Teórico é a média da informação processada até o momento
            double l_barra_teorico = informacaoAdaptativaAcumulada / totalSymbolsProcessed;
            
            // 4. Grava TODOS os pontos no CSV (Agora com 7 colunas)
            csvOut << totalSymbolsProcessed << "," 
                   << currentBPS << "," 
                   << accumulatedBPS << "," 
                   << entropia << ","
                   << entropiaOrdemK << ","
                   << order << ","
                   << l_barra_teorico << "\n";

            // Inicializa o suavizador na primeira janela
            if (smoothedBPS == 0.0) {
                smoothedBPS = currentBPS;
            }

            // A NOVA AVALIAÇÃO DE PERFORMANCE
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