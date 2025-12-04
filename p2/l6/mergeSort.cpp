#include <iostream>
#include <vector>
#include <cstring>
using namespace std;

struct pessoa{
    int matrícula;
    char nome[30];
    float nota;
};

void merge(std::vector<pessoa>& arr, int left, int mid, int right, int sortBy) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<pessoa> L(n1);
    std::vector<pessoa> R(n2);

    for (int i = 0; i < n1; ++i)
        L[i] = arr[left + i];
    for (int j = 0; j < n2; ++j)
        R[j] = arr[mid + 1 + j];

    int i = 0;
    int j = 0;
    int k = left;

    while (i < n1 && j < n2) {
        bool shouldTakeLeft = false;
        
        if (sortBy == 1) {
            // Sort by matrícula
            shouldTakeLeft = L[i].matrícula <= R[j].matrícula;
        } else if (sortBy == 2) {
            // Sort by nome
            shouldTakeLeft = strcmp(L[i].nome, R[j].nome) <= 0;
        } else if (sortBy == 3) {
            // Sort by nota
            shouldTakeLeft = L[i].nota <= R[j].nota;
        }
        
        if (shouldTakeLeft) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

void mergeSort(std::vector<pessoa>& arr, int left, int right, int sortBy) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        mergeSort(arr, left, mid, sortBy);
        mergeSort(arr, mid + 1, right, sortBy);

        merge(arr, left, mid, right, sortBy);
    }
}

int main() {
    std::vector<pessoa> alunos = {
        {1001, "Maria", 85.5},
        {1002, "João", 92.0},
        {1003, "Ana", 78.0},
        {1004, "Carlos", 88.5},
        {1005, "Beatriz", 91.0}
    };

    cout << "Escolha o campo para ordenar:\n";
    cout << "1. Matrícula\n";
    cout << "2. Nome\n";
    cout << "3. Nota\n";
    int choice;
    cin >> choice;

    switch(choice) {
        case 1:
            // Ordenar por matrícula
            mergeSort(alunos, 0, alunos.size() - 1, choice);
            break;
        case 2:
            // Ordenar por nome
            mergeSort(alunos, 0, alunos.size() - 1, choice);
            break;
        case 3:
            // Ordenar por nota
            mergeSort(alunos, 0, alunos.size() - 1, choice);
            break;
        default:
            cout << "Opção inválida!\n";
            return 1;
    }

    // Exibir lista ordenada
    cout << "Lista de alunos ordenada:\n";
    for (const auto& aluno : alunos) {
        cout << "Matrícula: " << aluno.matrícula << ", Nome: " << aluno.nome << ", Nota: " << aluno.nota << endl;
    }

    return 0;
}