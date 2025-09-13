#include <iostream>
#include <vector>
using namespace std;

class questao910 {
public:
    int size;
    std::vector<int> vet;

    questao910() {
        cout << "Digite o tamanho do vetor: ";
        cin >> size;
        cout << "Digite os valores do vetor: " << endl;
        int value;
        for(int i = 0; i < size; i++) {
            cout << "[" << i+1 << "]: ";
            cin >> value;
            vet.push_back(value);
        }
        bubbleSort();
        printVector();
    }
    
    void bubbleSort(){
        for(int n = size; n > 1; n--){
            for (int i = 0; i < n - 1; i++) {
                if (vet[i] < vet[i + 1]) { //ordenação decrescente
                    int temp = vet[i];
                    vet[i] = vet[i + 1];
                    vet[i + 1] = temp;
                }
            }
        }
    }

    void printVector() {
        cout << "Vetor ordenado: ";
        for (int i = 0; i < size; i++) {
            cout << vet[i] << " ";
        }
        cout << endl;
    }
    
    //questão 10
    //durante a questão 1-6, a busca binária foi muito mais rápida que as outras, por isso usei ela.
    int binSearch(int value, int& nSearches) {
        int left = 0;
        int right = size - 1;
        while (left <= right) {
            nSearches++;
            int mid = left + (right - left) / 2;
            if (vet[mid] == value) {
                return mid;
            } else if (vet[mid] > value) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        return -1;
    }
};

int main(){
    questao910 q;

    int value;
    cout << "Digite um valor para buscar: ";
    cin >> value;
    int nSearches = 0;
    int pos = q.binSearch(value, nSearches);
    if (pos != -1) {
        cout << "Valor encontrado na posição " << pos << endl;
    } else {
        cout << "Valor não encontrado" << endl;
    }
    cout << "Número de buscas: " << nSearches << endl;
    return 0;
}
    