#include "questao16.hpp"
#include <iostream>
using namespace std;

int main(){
    searchAed s;
    s.populateRandom();
    int value;

    cout << "Vetor: \n";
    s.printVector();
    
    s.bubbleSort();
    cout << "Vetor ordenado: \n";
    s.printVector();
    
    cout << "Digite um valor: ";
    cin >> value;
    
    //questão 6
    cout << "Realizando métodos de pesquisa...\n";
    int seq, bin, sent;
    seq = 0;
    bin = 0;
    sent = 0;
    
    int pos = s.pesqSec(value, seq); //Só precisamos da posição uma vez
    s.pesqBin(value, bin);
    s.pesqSent(value, sent);
    
    if(pos == -1){
        cout << "Valor nao encontrado" << endl;
    }else{
        cout << "Valor encontrado na posicao " << pos << endl;
    }
    
    cout << "Busca sequencial: " << seq << endl;
    cout << "Busca binaria: " << bin << endl;
    cout << "Busca sentinela: " << sent << endl;
    
    return 0;
}

