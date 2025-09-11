#include "lista3.hpp"
#include <iostream>
using namespace std;

int main(){
    search s;
    s.populateRandom();
    int value;
    cout << "Digite um valor: ";
    cin >> value;
    int pos = s.linearSearch(value);
    if(pos == -1){
        cout << "Valor nao encontrado" << endl;
    }else{
        cout << "Valor encontrado na posicao " << pos << endl;
    }
    return 0;
}

