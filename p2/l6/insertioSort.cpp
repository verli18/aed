//insertion sort para nomes

#include <iostream>
#include <vector>

void insertionSort(std::vector<std::string>& names) {
    int n = names.size();
    for (int i = 1; i < n; ++i) {
        std::string key = names[i];
        int j = i - 1;

        // Move elements of names[0..i-1], that are greater than key,
        // to one position ahead of their current position
        while (j >= 0 && names[j] > key) {
            names[j + 1] = names[j];
            j = j - 1;
        }
        names[j + 1] = key;
    }
}

int main() {
    std::vector<std::string> names = {"Maria", "João", "Ana", "Carlos", "Beatriz"};

    insertionSort(names);

    std::cout << "Nomes ordenados:\n";
    for (const auto& name : names) {
        std::cout << name << "\n";
    }

    return 0;
}