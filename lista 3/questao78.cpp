#include <vector>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
using namespace std;

class questao78 {
public:
    int size;
    std::vector<int> vet;
    questao78() {
        srand(time(NULL));
        cout << "Digite o tamanho do vetor: ";
        cin >> size;
        for(int i = 0; i < size; i++) {
            vet.push_back(rand() % 4096);
        }        
    }

    //questão 7
    int findSmallest(){
        int smallest = vet[0];
        for(int i = 1; i < size; i++) {
            if(vet[i] < smallest) {
                smallest = vet[i];
            }
        }
        return smallest;
    }

    //questão 8
    int findLargest(){
        int largest = vet[0];
        for(int i = 1; i < size; i++) {
            if(vet[i] > largest) {
                largest = vet[i];
            }
        }
        return largest;
    }

    void printVector() {
        int rows = (size + 15) / 16;
        for (int i = 0; i < rows; i++) {
            int startIdx = i * 16;
            int endIdx = std::min(startIdx + 16, size);
            cout << "[" << setw(3) << startIdx << " - " << setw(3) << endIdx - 1 << "]";
            for (int j = startIdx; j < endIdx; j++) {
                cout << " " << setw(4) << setfill('0') << vet[j];
            }
            cout << endl;
        }
        cout << endl;
    }

};

int main(){
    questao78 q;
    cout << "Vetor: \n";
    q.printVector();
    
    cout << "Menor valor: " << q.findSmallest() << endl;
    cout << "Maior valor: " << q.findLargest() << endl;
    
    return 0;
}