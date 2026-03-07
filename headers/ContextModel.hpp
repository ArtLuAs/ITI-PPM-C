#ifndef CONTEXTMODEL_HPP
#define CONTEXTMODEL_HPP

#include <deque>
#include <vector>
#include <memory>
#include <cstdint>
#include "TrieNode.hpp"

class ContextModel {
private:
    int maxOrder; // Ordem máxima (k) do nosso modelo PPM
    std::unique_ptr<TrieNode> root; // O contexto global de Ordem-0 (raiz da árvore)
    std::deque<uint32_t> history; // Janela deslizante que guarda os últimos 'k' caracteres

public:
    // Construtor: recebe o tamanho máximo do contexto
    ContextModel(int k);

    // Retorna uma lista de nós ativos (do contexto MAIOR para o MENOR)
    // Ex: Se o histórico é "AB", retorna os nós "AB", "B" e "" (Raiz).
    std::vector<TrieNode*> getActiveContextNodes() const;

    // Atualiza as frequências na árvore e avança a janela deslizante
    void updateAndShift(uint32_t symbol);

    // resetar a árvore
    void reset();
};

#endif // CONTEXTMODEL_HPP