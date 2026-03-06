#ifndef TRIENODE_HPP
#define TRIENODE_HPP

#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include "tabelaFrequencia.hpp" 

class TrieNode {
public:
    // Variáveis
    std::unique_ptr<SimpleFrequencyTable> freqTable;
    std::unordered_map<uint32_t, std::unique_ptr<TrieNode>> children;
    std::vector<uint32_t> activeSymbols;

    // Construtor e Métodos (sem a implementação aqui)
    TrieNode();
    void incrementSymbol(uint32_t symbol);
    TrieNode* getChild(uint32_t symbol) const;
    TrieNode* createChild(uint32_t symbol);
};

#endif // TRIENODE_HPP