#include <cstdlib>
#include <ctime>
#include <vector>
#include <iostream>
using namespace std;

class BubbleSort{
    public:
        int tamanho;
        std::vector<int> vetor;
        
        //construtor, cria e popula o vetor
        BubbleSort(int n){ // n é o número de elementos do vetor
            int temp;
            tamanho = n;

            //valores são inseridos no vetor
            for(int i = 0; i < n; i++){
                cout << "Digite o valor para a posição " << i << " do vetor: ";
                cin >> temp;
                vetor.push_back(temp);
            }

            cout << "vetor antes da ordenação: [ ";
            for(int i = 0; i < n; i++){ cout << vetor[i] << " "; }
            cout << "]" << endl;

        }

        void bubbleSort() {
            int n = vetor.size();
            bool troca;
            
            for (int i = 0; i < n-1; i++) {
                troca = false;
                for (int j = 0; j < n - i - 1; j++) {
                    if (vetor[j] > vetor[j+1]) { //se n-1 for maior que n, troque a posição dos dois

                        swap(vetor[j], vetor[j+1]);
                        troca = true;
                    }
                }

                if (!troca) // se não houve nenhuma troca, o vetor está ordenado.
                    break;
            }

            cout << "vetor após ordenação: [ ";
            for(int i = 0; i < n; i++){ cout << vetor[i] << " "; }
            cout << "]" << endl;
        }
};

int main(){
    BubbleSort sort(5);
    sort.bubbleSort();

    return 0;
}

