#ifndef PPM_HPP
#define PPM_HPP

#include <string>

// Declaração das funções principais do nosso compressor
void compressPPM(const std::string& input, const std::string& outputFile, int order);
void decompressPPM(const std::string& inputFile, const std::string& outputFile, int order);

#endif // PPM_HPP