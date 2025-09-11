#include <cstdint>
#include <string>
#include <vector>
#include <map>

typedef struct cor{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} cor;

typedef struct entidadeCarro {
    std::string placa;
    std::string telefone;
    cor cor;
    std::string marca; // não sabemos o tamanho da string
} entidadeCarro;

class gerenciadorEstacionamento{
    public:
        gerenciadorEstacionamento();
        ~gerenciadorEstacionamento();

        int nCarros;

        entidadeCarro getCar(std::string placa);
        std::string adicionarCarro( entidadeCarro carro);
        int removerCarro( std::string placa);

    private:
        std::map<std::string, entidadeCarro>carrosEstacionados;
};