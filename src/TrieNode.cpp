#include "../headers/TrieNode.hpp"

// Construtor
TrieNode::TrieNode() {
    // Inicializa o vetor com 257 posições zeradas 
    // (0 a 255 para os bytes normais, 256 para o Escape/EOF)
    // // 0-255 (Chars), 256 (Escape), 257 (EOF)
    std::vector<uint32_t> initialFreqs(258, 0);
    
    // O Escape (256) DEVE começar com peso > 0.
    initialFreqs[256] = 1; 
    
    freqTable = std::make_unique<SimpleFrequencyTable>(initialFreqs);
}

// Incrementa a frequência de um símbolo e atualiza a lista de ativos
void TrieNode::incrementSymbol(uint32_t symbol) {
    if (freqTable->get(symbol) == 0) {
        activeSymbols.push_back(symbol);
    }
    freqTable->increment(symbol);
}

// Retorna o nó filho correspondente ao símbolo, ou nullptr se não existir
TrieNode* TrieNode::getChild(uint32_t symbol) const {
    auto it = children.find(symbol);
    if (it != children.end()) {
        return it->second.get();
    }
    return nullptr;
}

// Cria um novo nó filho para o símbolo e retorna o ponteiro para ele
TrieNode* TrieNode::createChild(uint32_t symbol) {
    children[symbol] = std::make_unique<TrieNode>();
    return children[symbol].get();
}