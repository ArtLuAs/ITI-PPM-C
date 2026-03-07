#ifndef TABELAFREQUENCIA_HPP
#define TABELAFREQUENCIA_HPP

#include <cstdint>
#include <vector>
#include <utility> // Necessário para std::pair

// Interface abstrata base
class FrequencyTable {
public:
    virtual ~FrequencyTable() = 0;
    virtual std::uint32_t getSymbolLimit() const = 0;
    virtual std::uint32_t get(std::uint32_t symbol) const = 0;
    virtual void set(std::uint32_t symbol, std::uint32_t freq) = 0;
    virtual void increment(std::uint32_t symbol) = 0;
    virtual std::uint32_t getTotal() const = 0;
    virtual std::uint32_t getLow(std::uint32_t symbol) const = 0;
    virtual std::uint32_t getHigh(std::uint32_t symbol) const = 0;
};

// Tabela Estática
class FlatFrequencyTable : public FrequencyTable {
private:
    std::uint32_t numSymbols;

public:
    explicit FlatFrequencyTable(std::uint32_t numSyms);
    std::uint32_t getSymbolLimit() const override;
    std::uint32_t get(std::uint32_t symbol) const override;
    std::uint32_t getTotal() const override;
    std::uint32_t getLow(std::uint32_t symbol) const override;
    std::uint32_t getHigh(std::uint32_t symbol) const override;
    void set(std::uint32_t symbol, std::uint32_t freq) override;
    void increment(std::uint32_t symbol) override;

private:
    void checkSymbol(std::uint32_t symbol) const;
};

// Tabela Dinâmica baseada em Fenwick Tree
class SimpleFrequencyTable : public FrequencyTable {
private:
    std::vector<std::uint32_t> frequencies;
    std::vector<std::uint32_t> tree; // Árvore de Fenwick
    std::uint32_t total;
    
    // Para guardar as frequências temporariamente removidas pelo Princípio de Exclusão
    std::vector<std::pair<std::uint32_t, std::uint32_t>> savedFrequencies;

public:
    explicit SimpleFrequencyTable(const std::vector<std::uint32_t> &freqs);
    explicit SimpleFrequencyTable(const FrequencyTable &freqs);
    
    std::uint32_t getSymbolLimit() const override;
    std::uint32_t get(std::uint32_t symbol) const override;
    void set(std::uint32_t symbol, std::uint32_t freq) override;
    void increment(std::uint32_t symbol) override;
    std::uint32_t getTotal() const override;
    std::uint32_t getLow(std::uint32_t symbol) const override;
    std::uint32_t getHigh(std::uint32_t symbol) const override;

    // Métodos para o Princípio de Exclusão (PPM)
    void excludeSymbol(std::uint32_t symbol);
    void restoreExcludedSymbols();

    // Método para evitar overflow e aplicar o fator de envelhecimento (Aging)
    void halveFrequencies();

private:
    std::uint32_t read(std::uint32_t index) const;
    static std::uint32_t checkedAdd(std::uint32_t x, std::uint32_t y);
};

#endif // TABELAFREQUENCIA_HPP