#include <iostream>
#include <string>
using namespace std;

struct node {
    string nomeFuncionario;
    int nVendas;
    node* prev;
    node* next;
};

class DoubleLinkedList {
private:
    node* head;
    node* tail;
public:
    DoubleLinkedList() : head(nullptr), tail(nullptr) {}

    void insert(const string& nome, int vendas) {
        node* newNode = new node{ nome, vendas, nullptr, nullptr };
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
    }

    void removeAt(int position) {
        if (position < 0 || !head) return;
        node* current = head;
        int index = 0;
        while (current && index < position) {
            current = current->next;
            index++;
        }
        if (!current) return; // Position out of bounds
        if (current->prev) current->prev->next = current->next;
        else head = current->next; // Removing head
        if (current->next) current->next->prev = current->prev;
        else tail = current->prev; // Removing tail
        delete current;
    }

    void displayTopSeller() const {
        if (!head) {
            cout << "Lista vazia." << endl;
            return;
        }
        node* current = head;
        node* topSeller = head;
        while (current) {
            if (current->nVendas > topSeller->nVendas) {
                topSeller = current;
            }
            current = current->next;
        }
        cout << "Funcionario com maior numero de vendas: " << topSeller->nomeFuncionario 
             << " com " << topSeller->nVendas << " vendas." << endl;
    }

    ~DoubleLinkedList() {
        node* current = head;
        while (current) {
            node* next = current->next;
            delete current;
            current = next;
        }
    }
};

int main() {
    DoubleLinkedList lista;
    lista.insert("Funcionario 1", 10);
    lista.insert("Funcionario 2", 20);
    lista.insert("Funcionario 3", 30);

    bool shouldClose = false;
    int option;
    while(!shouldClose){
        cout << "\nMenu:\n1. Inserir Funcionario\n2. Remover Funcionario na Posicao\n3. Mostrar Funcionario com Maior Vendas\n4. Sair\nEscolha uma opcao: ";
        cin >> option;
        cin.ignore();

        switch (option) {
            case 1: {
                string nome;
                int vendas;
                cout << "Digite o nome do funcionario: ";
                getline(cin, nome);
                cout << "Digite o numero de vendas: ";
                cin >> vendas;
                cin.ignore();
                lista.insert(nome, vendas);
                break;
            }
            case 2: {
                int pos;
                cout << "Digite a posicao do funcionario a ser removido: ";
                cin >> pos;
                cin.ignore();
                lista.removeAt(pos);
                break;
            }
            case 3:
                lista.displayTopSeller();
                break;
            case 4:
                shouldClose = true;
                break;
            default:
                cout << "Opcao invalida." << endl;
        }
    }

    return 0;
}