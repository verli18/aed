//implementação de uma árvore binária de busca

#include <iostream>
#include <memory>
#include <string>
using namespace std;

struct Node {
    int ID;
    string nome;
    float preço;
    Node* left;
    Node* right;
};


class BinaryTree {
private:
    Node* root;
    void insert(Node*& node, int id, const string& nome, float preço) {
        if (!node) {
            node = new Node{id, nome, preço, nullptr, nullptr};
        } else if (id < node->ID) {
            insert(node->left, id, nome, preço);
        } else if (id > node->ID) {
            insert(node->right, id, nome, preço);
        } else {
            cout << "ID já existe na árvore: " << id << endl;
        }
    }
    void inOrder(const Node* node) const {
        if (node) {
            inOrder(node->left);
            cout << "ID: " << node->ID << ", Nome: " << node->nome << ", Preço: " << node->preço << endl;
            inOrder(node->right);
        }
    }
public:
    BinaryTree() : root(nullptr) {}
    void insert(int id, const string& nome, float preço) {
        insert(root, id, nome, preço);
    }
    void inOrder() const {  
        inOrder(root);
    }
    Node* search(int id) const {
        Node* current = root;
        while (current) {
            if (id == current->ID) {
                return current;
            } else if (id < current->ID) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        throw runtime_error("ID não encontrado na árvore: " + to_string(id));
    }
};

int main() {
    BinaryTree tree;
    tree.insert(3, "Item3", 30.0);
    tree.insert(1, "Item1", 10.0);
    tree.insert(4, "Item4", 40.0);
    tree.insert(2, "Item2", 20.0);

    cout << "choose an option:\n1 - Search ID\n2 - Display items in order\n3 - Insert new item\n4 - Exit\n";
    int option;
    cin >> option;

   switch (option) {
       case 1: {
           cout << "Enter ID to search: ";
           int searchID;
           cin >> searchID;
              try {
                Node* result = tree.search(searchID);
                cout << "Found - ID: " << result->ID << ", Nome: " << result->nome << ", Preço: " << result->preço << endl;
              } catch (const runtime_error& e) {
                cout << e.what() << endl;
              }
           // Implement search functionality
           break;
       }
       case 2:
           cout << "Items in order:" << endl;
           tree.inOrder();
           break;
       case 3: {
           int newID;
           string newName;
           float newPrice;
           cout << "Enter new item ID: ";
           cin >> newID;
           cout << "Enter new item Name: ";
           cin >> newName;
           cout << "Enter new item Price: ";
           cin >> newPrice;
           tree.insert(newID, newName, newPrice);
           break;
       }
       case 4:
           cout << "Exiting program." << endl;
           break;
       default:
           cout << "Invalid option." << endl;
           break;
   }

   return 0;
}