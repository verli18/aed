//double linked list exercise, air traffic control
#include <iostream>
#include <stack>
#include <string>
#include <ctime>
using namespace std;

struct Node {
    string airplaneID;
    string airplaneModel;
    time_t expectedLandingTime;
    Node* prev;
    Node* next;
};

class DoubleLinkedList {
private:
    Node* head;
    Node* tail;
public:
    DoubleLinkedList() : head(nullptr), tail(nullptr) {}
    void insert(const string& id, const string& model, time_t landingTime) {
        Node* newNode = new Node{ id, model, landingTime, nullptr, nullptr };
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
    }
    void displayStartToEnd() const {
        Node* current = head;
        while (current) {
            cout << "Airplane ID: " << current->airplaneID 
                 << ", Model: " << current->airplaneModel 
                 << ", Expected Landing Time: " << ctime(&current->expectedLandingTime);
            current = current->next;
        }
    }

    void displayEndToStart() const {
        Node* current = tail;
        while (current) {
            cout << "Airplane ID: " << current->airplaneID 
                 << ", Model: " << current->airplaneModel 
                 << ", Expected Landing Time: " << ctime(&current->expectedLandingTime);
            current = current->prev;
        }
    }

    void displayAtPosition(int position) const {
        if (position < 0) {
            cout << "Invalid position." << endl;
            return;
        }
        Node* current = head;
        int index = 0;
        while (current && index < position) {
            current = current->next;
            index++;
        }
        if (current) {
            cout << "Airplane ID: " << current->airplaneID 
                 << ", Model: " << current->airplaneModel 
                 << ", Expected Landing Time: " << ctime(&current->expectedLandingTime);
        } else {
            cout << "Position out of bounds." << endl;
        }
    }

    void removeAtStart() {
        if (!head) return;
        Node* temp = head;
        head = head->next;
        if (head) head->prev = nullptr;
        else tail = nullptr; // List became empty
        delete temp;
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
    cout << "Air Traffic Control System" << endl;

    bool shouldClose = false;
    int option;
    while(!shouldClose){
        cout << "\nMenu:\n1. Insert Airplane\n2. Display from Start to End\n3. Display from End to Start\n4. Display at Position\n5. Remove at Start\n6. Exit\nChoose an option: ";
        cin >> option;
        cin.ignore();

        switch(option){
            case 1: {
                string id, model;
                time_t landingTime;
                cout << "Enter Airplane ID: ";
                getline(cin, id);
                cout << "Enter Airplane Model: ";
                getline(cin, model);
                cout << "Enter Expected Landing Time (as epoch time): ";
                cin >> landingTime;
                cin.ignore();
                list.insert(id, model, landingTime);
                break;
            }
            case 2:
                list.displayStartToEnd();
                break;
            case 3:
                list.displayEndToStart();
                break;
            case 4: {
                int position;
                cout << "Enter position to display: ";
                cin >> position;
                cin.ignore();
                list.displayAtPosition(position);
                break;
            }
            case 5:
                list.removeAtStart();
                cout << "Removed airplane at start of the list." << endl;
                break;
            case 6:
                shouldClose = true;
                break;
            default:
                cout << "Invalid option. Please try again." << endl;
        }
    }

    return 0;
}