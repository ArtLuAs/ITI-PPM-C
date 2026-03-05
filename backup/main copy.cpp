// #include <iostream>
// #include <fstream>
// #include "ArithmeticCoder.hpp"
// #include "tabelaFrequencia.hpp"

// using namespace std;

// int main() {

//     ofstream outStream("teste_comprimido.bin", ios::binary);
//     BitOutputStream bitOut(outStream);
//     ArithmeticEncoder encoder(32, bitOut);

//     string texto = "Compressao de teste concluida com sucesso!";

//     // Cria vetor zerado
//     vector<uint32_t> frequencias(257, 0);

//     // Conta todas as ocorrências (modelo semi-adaptativo)
//     for (unsigned char c : texto) {
//         frequencias[c]++;
//     }

//     // EOF precisa ter pelo menos 1
//     frequencias[256] = 1;

//     // Agora sim criamos a tabela
//     SimpleFrequencyTable freqTable(frequencias);

//     // Codificamos
//     for (unsigned char c : texto) {
//         encoder.write(freqTable, c);
//     }

//     // 6️⃣ EOF
//     encoder.write(freqTable, 256);

//     encoder.finish();
//     bitOut.finish();
//     outStream.close();

//     cout << "Compressao de teste concluida com sucesso!" << endl;
//     return 0;
// }