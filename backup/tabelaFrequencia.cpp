#include <stdexcept>
#include "tabelaFrequencia.hpp"

using std::uint32_t;

FrequencyTable::~FrequencyTable() {}

FlatFrequencyTable::FlatFrequencyTable(uint32_t numSyms) :
		numSymbols(numSyms) {
	if (numSyms < 1)
		throw std::domain_error("Number of symbols must be positive");
}

uint32_t FlatFrequencyTable::getSymbolLimit() const {
	return numSymbols;
}

uint32_t FlatFrequencyTable::get(uint32_t symbol) const  {
	checkSymbol(symbol);
	return 1;
}

uint32_t FlatFrequencyTable::getTotal() const  {
	return numSymbols;
}

uint32_t FlatFrequencyTable::getLow(uint32_t symbol) const  {
	checkSymbol(symbol);
	return symbol;
}

uint32_t FlatFrequencyTable::getHigh(uint32_t symbol) const  {
	checkSymbol(symbol);
	return symbol + 1;
}

void FlatFrequencyTable::set(uint32_t, uint32_t)  {
	throw std::logic_error("Unsupported operation");
}

void FlatFrequencyTable::increment(uint32_t) {
	throw std::logic_error("Unsupported operation");
}

void FlatFrequencyTable::checkSymbol(uint32_t symbol) const {
	if (symbol >= numSymbols)
		throw std::domain_error("Symbol out of range");
}

// -------------------------------------------------------------------------
// SimpleFrequencyTable - Implementação com Fenwick Tree
// -------------------------------------------------------------------------

SimpleFrequencyTable::SimpleFrequencyTable(const std::vector<uint32_t> &freqs) {
	if (freqs.size() > UINT32_MAX - 1)
		throw std::length_error("Too many symbols");
	uint32_t size = static_cast<uint32_t>(freqs.size());
	if (size < 1)
		throw std::invalid_argument("At least 1 symbol needed");
	
	frequencies = freqs;
	tree.assign(size + 1, 0);
	total = 0;
	
	// Construção em O(N) da Fenwick Tree
	for (uint32_t i = 0; i < size; i++) {
		total = checkedAdd(total, frequencies[i]);
		tree[i + 1] = checkedAdd(tree[i + 1], frequencies[i]);
	}
	for (uint32_t i = 1; i <= size; i++) {
		uint32_t parent = i + (i & -i);
		if (parent <= size) {
			tree[parent] = checkedAdd(tree[parent], tree[i]);
		}
	}
}

SimpleFrequencyTable::SimpleFrequencyTable(const FrequencyTable &freqs) {
	uint32_t size = freqs.getSymbolLimit();
	if (size < 1)
		throw std::invalid_argument("At least 1 symbol needed");
	if (size > UINT32_MAX - 1)
		throw std::length_error("Too many symbols");
	
	frequencies.reserve(size);
	tree.assign(size + 1, 0);
	total = 0;
	
	for (uint32_t i = 0; i < size; i++) {
		uint32_t freq = freqs.get(i);
		frequencies.push_back(freq);
		total = checkedAdd(total, freq);
		tree[i + 1] = checkedAdd(tree[i + 1], freq);
	}
	
	// Construção em O(N) da Fenwick Tree
	for (uint32_t i = 1; i <= size; i++) {
		uint32_t parent = i + (i & -i);
		if (parent <= size) {
			tree[parent] = checkedAdd(tree[parent], tree[i]);
		}
	}
}

uint32_t SimpleFrequencyTable::getSymbolLimit() const {
	return static_cast<uint32_t>(frequencies.size());
}

uint32_t SimpleFrequencyTable::get(uint32_t symbol) const {
	return frequencies.at(symbol);
}

void SimpleFrequencyTable::set(uint32_t symbol, uint32_t freq) {
	uint32_t old_freq = frequencies.at(symbol);
	if (freq == old_freq) return;
	
	if (freq > old_freq) {
		uint32_t delta = freq - old_freq;
		total = checkedAdd(total, delta);
		for (uint32_t i = symbol + 1; i < tree.size(); i += i & -i) {
			tree[i] = checkedAdd(tree[i], delta);
		}
	} else {
		uint32_t delta = old_freq - freq;
		total -= delta; // Como total >= old_freq, não causará underflow
		for (uint32_t i = symbol + 1; i < tree.size(); i += i & -i) {
			tree[i] -= delta;
		}
	}
	frequencies[symbol] = freq;
}

void SimpleFrequencyTable::increment(uint32_t symbol) {
	if (frequencies.at(symbol) == UINT32_MAX)
		throw std::overflow_error("Arithmetic overflow");
	
	total = checkedAdd(total, 1);
	frequencies[symbol]++;
	
	for (uint32_t i = symbol + 1; i < tree.size(); i += i & -i) {
		tree[i] = checkedAdd(tree[i], 1);
	}
}

uint32_t SimpleFrequencyTable::getTotal() const {
	return total;
}

uint32_t SimpleFrequencyTable::getLow(uint32_t symbol) const {
	return read(symbol);
}

uint32_t SimpleFrequencyTable::getHigh(uint32_t symbol) const {
	return read(symbol + 1);
}

uint32_t SimpleFrequencyTable::read(uint32_t index) const {
	uint32_t sum = 0;
	for (uint32_t i = index; i > 0; i -= i & -i) {
		sum = checkedAdd(sum, tree[i]);
	}
	return sum;
}

uint32_t SimpleFrequencyTable::checkedAdd(uint32_t x, uint32_t y) {
	if (x > UINT32_MAX - y)
		throw std::overflow_error("Arithmetic overflow");
	return x + y;
}