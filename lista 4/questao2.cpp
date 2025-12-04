#include <stack>
#include <vector>
#include <iostream>
using namespace std;

struct products{
    int id;
    string name;
    float weight;
};

class logisticalCenter{
    stack<products> productStack;
    public:
        void addProduct(int id, string name, float weight){
            productStack.push({id, name, weight});
        }
        void shipProduct(){
            if(!productStack.empty()){
                products p = productStack.top();
                cout << "Shipping product ID: " << p.id << ", Name: " << p.name << ", Weight: " << p.weight << "kg" << endl;
                productStack.pop();
            } else {
                cout << "No products to ship." << endl;
            }
        }
        bool isEmpty(){
            return productStack.empty();
        }
};

int main(){
    cout << "Logistical Center Product Management" << endl;
    logisticalCenter center;
    int option;
    
    cout << "\nMenu:\n1. Add Product\n2. Ship Product\n3. Exit\nChoose an option: ";
    cin >> option;
    cin.ignore();

    bool shouldClose = false;
    while(!shouldClose){
        switch(option){
            case 1: {
                int id;
                string name;
                float weight;
                cout << "Enter product ID: ";
                cin >> id;
                cin.ignore();
                cout << "Enter product name: ";
                getline(cin, name);
                cout << "Enter product weight (kg): ";
                cin >> weight;
                center.addProduct(id, name, weight);
                break;
            }
            case 2:
                center.shipProduct();
                break;
            case 3:
                shouldClose = true;
                break;
            default:
                cout << "Invalid option. Please try again." << endl;
        }
        if (!shouldClose) {
            cout << "\nMenu:\n1. Add Product\n2. Ship Product\n3. Exit\nChoose an option: ";
            cin >> option;
            cin.ignore();
        }
    }

    cout << "Exiting the system." << endl;
    return 0;
}