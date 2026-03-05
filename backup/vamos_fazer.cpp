// isso aqui deve ser na freqneucy table
// eh a ideia do principio da exclusao

//FEITO

// class SimpleFrequencyTable {
//     // ... seus métodos atuais ...

// private:
//     // Para guardar as frequências temporariamente removidas pelo Princípio de Exclusão
//     std::vector<std::pair<uint32_t, uint32_t>> savedFrequencies;

// public:
//     // Zera a frequência de um símbolo, mas lembra qual era o valor original
//     void excludeSymbol(uint32_t symbol) {
//         uint32_t currentFreq = get(symbol);
//         if (currentFreq > 0) {
//             savedFrequencies.push_back({symbol, currentFreq});
//             set(symbol, 0); // Sua Fenwick Tree já atualiza o total e os nós corretamente aqui!
//         }
//     }

//     // Restaura todos os símbolos excluídos para seus valores originais
//     void restoreExcludedSymbols() {
//         for (const auto& pair : savedFrequencies) {
//             set(pair.first, pair.second); // Restaura na Fenwick Tree
//         }
//         savedFrequencies.clear();
//     }
// };