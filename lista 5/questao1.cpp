#include <iostream>
#include <stack>
#include <string>
#include <ctime>

using namespace std;

struct Node {
    string website;
    time_t timestamp;
    Node* prev;
    Node* next;
};

class DoubleLinkedList {
private:
    Node* head;
    Node* tail;
public:
    DoubleLinkedList() : head(nullptr), tail(nullptr) {}

    void insert(const string& value) {
        Node* newNode = new Node{ value, time(nullptr), nullptr, nullptr };
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
    }

    void display() const {
        Node* current = head;
        while (current) {
            cout << "Value: " << current->website << ", Timestamp: " << ctime(&current->timestamp);
            current = current->next;
        }
    }

    ~DoubleLinkedList() {
        Node* current = head;
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }
};

int main() {
    DoubleLinkedList list;
    cout << "Search history manager" << endl;

    bool shouldClose = false;
    int option;
    while(!shouldClose){
        cout << "\nMenu:\n1. Insert Website\n2. Display History\n3. Exit\nChoose an option: ";
        cin >> option;
        cin.ignore();

        switch(option){
            case 1: {
                string website;
                cout << "Enter website URL: ";
                getline(cin, website);
                list.insert(website);
                break;
            }
            case 2:
                list.display();
                break;
            case 3:
                shouldClose = true;
                break;
            default:
                cout << "Invalid option. Please try again." << endl;
        }
    }

    return 0;
}