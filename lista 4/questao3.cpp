#include <iostream>
#include <queue>
#include <vector>
using namespace std;

struct pedido{
    int id;
    string pedido;
};
class atendimento{

    private:
        queue<pedido> fila;
    public:
        void adicionarPedido(int id, string pedido){
            fila.push({id, pedido});
        }
        void processarPedido(){
            if(!fila.empty()){
                pedido p = fila.front();
                cout << "Processando pedido ID: " << p.id << ", Pedido: " << p.pedido << endl;
                fila.pop();
            } else {
                cout << "Nenhum pedido na fila para processar." << endl;
            }
        }
        bool estaVazia(){
            return fila.empty();
        }

};

int main(){
    cout << "Sistema de Atendimento de Pedidos" << endl;
    atendimento atendimentoCliente;
    int opcao;
    
    cout << "\nMenu:\n1. Adicionar Pedido\n2. Processar Pedido\n3. Sair\nEscolha uma opcao: ";
    cin >> opcao;
    cin.ignore();

    bool shouldClose = false;
    while(!shouldClose){
        switch(opcao){
            case 1: {
                int id;
                string pedido;
                cout << "Digite o ID do pedido: ";
                cin >> id;
                cin.ignore();
                cout << "Digite o descricao do pedido: ";
                getline(cin, pedido);
                atendimentoCliente.adicionarPedido(id, pedido);
                break;
            }
            case 2:
                atendimentoCliente.processarPedido();
                break;
            case 3:
                shouldClose = true;
                break;
            default:
                cout << "Opcao invalida. Tente novamente." << endl;
        }
    }

    return 0;
}