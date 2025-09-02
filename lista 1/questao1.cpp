#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;
//agenda telefônica

struct contato{
    string nome;
    string telefone;
};

class agendaTelefônica {
    public:
        agendaTelefônica(){
            nContatos = 0;
        }

        int nContatos;

        void adicionarContato(contato c){
            agenda.push_back(c);
            nContatos++;
        }

        contato getContato(int pos){
            if(pos < 0 || pos >= nContatos){
                printf("Posição inválida\n");
                return contato();
            }
            return agenda[pos];
        }

        void imprimirContato(int pos){
            if(pos < 0 || pos >= nContatos){
                printf("Posição inválida\n");
                return;
            }
            printf("Nome: %s\n", agenda[pos].nome.c_str());
            printf("Telefone: %s\n\n", agenda[pos].telefone.c_str());
        }

    private:
        vector<contato> agenda;
};


int main(){
    bool shouldClose = false;
    int op;
    agendaTelefônica agenda;
    contato contatoTeste;

    cout << "Agenda eletrônica, escolha uma opção: \n";
    cout << "1 - Adicionar contato\n";
    cout << "2 - Listar informações de um contato\n";
    cout << "3 - Sair\n";

    while(!shouldClose){

        cout << "Digite uma opção: ";
        cin >> op;

        switch(op){
            case 1:
                cout << "Digite o nome: ";
                cin >> contatoTeste.nome;

                cout << "Digite o telefone: ";
                cin >> contatoTeste.telefone;
                agenda.adicionarContato(contatoTeste);
                cout << "Contato salvo\n\n";
                break;
            case 2:
                int pos;
                cout << "Digite a posição do contato: ";
                cin >> pos;
                cout << "\n";
                agenda.imprimirContato(pos-1);
                break;
            case 3:
                shouldClose = true;
                break;
            default:
                cout << "Opção inválida\n\n";
                break;
        }
    }

    return 0;
}
