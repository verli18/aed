//double linked list ip list exercise 
#include <iostream>
using namespace std;

struct Node {
    string ipAddress;
    Node* prev;
    Node* next;
};

class DoubleLinkedList {
private:
    Node* head;
    Node* tail;
public:
    DoubleLinkedList() : head(nullptr), tail(nullptr) {}
    void insert(const string& ip) {
        Node* newNode = new Node{ ip, nullptr, nullptr };
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
            cout << "IP Address: " << current->ipAddress << endl;
            current = current->next;
        }
    }

    int isIpInList(const string& ip) const {
        Node* current = head;
        int n = 1;
        while (current) {
            if (current->ipAddress == ip) {
                return n;
            }
            current = current->next;
            n++;
        }
        return -1;
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
    DoubleLinkedList ipList;
    ipList.insert("192.168.1.1");
    ipList.insert("192.168.1.2");
    ipList.insert("192.168.1.3");
    ipList.insert("192.168.1.4");
    ipList.insert("192.168.1.5");

    bool shouldClose = false;
    int option;
    int address;
    string ipToCheck;
    while(!shouldClose){
        cout << "\nMenu:\n1. Insert IP Address\n2. Check if IP is in List\n3. Exit\nChoose an option: ";
        cin >> option;
        cin.ignore();

        switch(option) {
            case 1:
                cout << "Enter IP Address to insert: ";
                getline(cin, ipToCheck);
                ipList.insert(ipToCheck);
                break;
            case 2:
                cout << "Enter IP Address to check: ";
                getline(cin, ipToCheck);
                address = ipList.isIpInList(ipToCheck);
                if (address != -1) {
                    cout << "IP Address " << ipToCheck << " is in the address " << address << endl;
                } else {
                    cout << "IP Address " << ipToCheck << " is not in the list." << endl;
                }
                break;
            case 3:
                shouldClose = true;
                break;
            default:
                cout << "Invalid option. Please try again." << endl;
        }
    }
    ipList.display();

    return 0;
}