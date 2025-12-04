#include <iostream>
#include <string>
using namespace std;

class produto{
    public:
        //construtor, obrigatóriamente recebendo os valores necessários
        produto(string nomeConst, float preçoConst, int quantidadeConst){
            nome = nomeConst;
            preço = preçoConst;
            quantidade = quantidadeConst;
        }

        //métodos set e get
        string getNome(){ return nome;}
        float getPreço(){ return preço;}
        int getQuantidade(){ return quantidade;}

        void setNome(string nomeIn) { nome = nomeIn;}
        void setPreço(float preçoIn) { preço = preçoIn;}
        void setQuantidade(int quantidadeIn) { quantidade = quantidadeIn;}
        
        //calcula o valor e mostra no terminal
        float calcularValorTotal(){ return quantidade*preço;}
        void printProduto(){ cout << "Produto: " << nome << " | Valor em estoque: RS" << calcularValorTotal() << ".00" << endl;}
    private: 
        string nome;
        float preço;
        int quantidade;
};

int main(){
    produto lapis("Lápis", 1.0f, 35);
    produto caderno("caderno", 10.0f, 15);
    produto mochila("mochila", 80.0f, 3);
    
    lapis.printProduto();
    caderno.printProduto();
    mochila.printProduto();

    return 0;
}

