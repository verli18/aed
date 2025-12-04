#include <iostream>
#include <stack>
#include <string>

using namespace std;

class FraseContrario {
private:
    stack<char> pilha;

public:
    // Método para inverter a frase usando pilha
    string inverter(const string& frase) {
        // Limpa a pilha antes de usar
        while (!pilha.empty()) {
            pilha.pop();
        }

        // Empilha todos os caracteres da frase
        for (char c : frase) {
            pilha.push(c);
        }

        // Desempilha os caracteres para formar a frase invertida
        string fraseInvertida = "";
        while (!pilha.empty()) {
            fraseInvertida += pilha.top();
            pilha.pop();
        }

        return fraseInvertida;
    }

    // Método alternativo que recebe a frase por referência e a modifica
    void inverterInPlace(string& frase) {
        frase = inverter(frase);
    }
};

int main() {
    FraseContrario inversor;
    string frase;

    cout << "Digite uma frase para inverter: ";
    getline(cin, frase);

    string fraseInvertida = inversor.inverter(frase);

    cout << "\nFrase original: " << frase << endl;
    cout << "Frase invertida: " << fraseInvertida << endl;

    // Demonstração do método alternativo
    cout << "\n--- Demonstração do método inverterInPlace ---" << endl;
    string outraFrase;
    cout << "Digite outra frase: ";
    getline(cin, outraFrase);
    
    cout << "Antes: " << outraFrase << endl;
    inversor.inverterInPlace(outraFrase);
    cout << "Depois: " << outraFrase << endl;

    return 0;
}
