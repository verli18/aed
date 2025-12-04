#include <iostream>
#include <queue>
using namespace std;

struct Node{
    int n;
    //ponteiros para os dois nódulos filhos
    Node* left;
    Node* right;

    Node(int value) : n(value), left(nullptr), right(nullptr) {}
};

class arvore{
    public:
        Node* root; //começo da árvore
        arvore(){};

        Node* inserir(Node* node, int value) { //implementação com recursividade
            if (node == nullptr) {
                return new Node(value);
            }

            if (value < node->n) { //se o valor do nódulo atual for maior que o valor que queremos inserir, navegamas à esquerda, senão à direita
                node->left = inserir(node->left, value);
                } else if (value > node->n) {
                node->right = inserir(node->right, value);
            }

            return node;
        }    
    
        void recursaoInorder(Node* node) { //fazemos a recursão da esquerda para a direita ( menor ao maior)
            if (node != nullptr) {
                recursaoInorder(node->left);
                cout << node->n << " ";
                recursaoInorder(node->right);
            }
        }
};

int main(){
    arvore arvoreBin;

    arvoreBin.inserir(arvoreBin.root,7); //inserimos no começo da árvore
    arvoreBin.inserir(arvoreBin.root,4);
    arvoreBin.inserir(arvoreBin.root,9);
    arvoreBin.inserir(arvoreBin.root,1);
    arvoreBin.inserir(arvoreBin.root,6);

    arvoreBin.recursaoInorder(arvoreBin.root);
    return 0;
}    
