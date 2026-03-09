#include "../headers/ContextModel.hpp"
#include <stdexcept>

ContextModel::ContextModel(int k) : maxOrder(k) {
    if (k < 0) {
        throw std::invalid_argument("Erro: A ordem (k) do modelo PPM deve ser >= 0.");
    }

    root = std::make_unique<TrieNode>();
    
    // Ordem 0: 
    // A raiz precisa conhecer todos os símbolos possíveis do alfabeto (0 a 255).
    for (uint32_t i = 0; i < 256; i++) {
        root->incrementSymbol(i); 
    }

    root->incrementSymbol(257); // Garante o EOF
    root->incrementSymbol(258); // Garante o Reset
}

// Retorna os contextos do maior para o menor
std::vector<TrieNode*> ContextModel::getActiveContextNodes() const {
    std::vector<TrieNode*> nodes;
    
    // Iteramos gerando os sufixos do histórico atual.
    // Se history = ['A', 'B'], procuramos o caminho "A"->"B", e depois o caminho "B".
    for (size_t start = 0; start < history.size(); ++start) {
        TrieNode* current = root.get();
        bool found = true;
        
        for (size_t i = start; i < history.size(); ++i) {
            current = current->getChild(history[i]);
            if (current == nullptr) {
                found = false; // Este contexto ainda não existe na árvore
                break;
            }
        }
        
        if (found) {
            nodes.push_back(current);
        }
    }
    
    // Por fim, sempre adicionamos a Raiz (Ordem-0) como último recurso (fallback)
    nodes.push_back(root.get());
    
    return nodes;
}

// Atualiza a árvore e desliza a janela
void ContextModel::updateAndShift(uint32_t symbol) {
    
    // Atualizamos todos os sufixos do histórico atual.
    // Se o histórico é "AB" e lemos "X", precisamos incrementar "X" no nó "AB",
    // no nó "B" e no nó raiz "". Se o caminho "AB" não existir, criamos.
    
    for (size_t start = 0; start <= history.size(); ++start) {
        TrieNode* current = root.get();
        
        // Percorre a árvore para encontrar (ou criar) o nó de contexto
        for (size_t i = start; i < history.size(); ++i) {
            TrieNode* nextNode = current->getChild(history[i]);
            if (nextNode == nullptr) {
                nextNode = current->createChild(history[i]);
            }
            current = nextNode;
        }
        
        // Incrementa a frequência do caractere lido neste contexto
        current->incrementSymbol(symbol);
    }

    // Atualiza a janela deslizante
    history.push_back(symbol);
    
    // Mantém o tamanho máximo da janela igual a 'k'
    if (history.size() > static_cast<size_t>(maxOrder)) {
        history.pop_front(); 
    }
}

// resetar a árvore
void ContextModel::reset() {
    // Limpa a janela deslizante
    history.clear(); 
    
    // Destrói a árvore antiga e cria uma raiz nova
    root = std::make_unique<TrieNode>();
    
    // Readiciona os símbolos obrigatórios na Ordem-0
    for (uint32_t i = 0; i < 256; i++) {
        root->incrementSymbol(i); 
    }
    root->incrementSymbol(257); // Garante o EOF
    root->incrementSymbol(258); // Garante o RESET
}