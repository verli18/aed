//quick sort para ordenação de caracteres em uma string

#include <iostream>
#include <string>

void quickSort(std::string& str, int low, int high) {
    if (low < high) {
        char pivot = str[high];
        int i = low - 1;

        for (int j = low; j < high; ++j) {
            if (str[j] < pivot) {
                ++i;
                std::swap(str[i], str[j]);
            }
        }
        std::swap(str[i + 1], str[high]);
        int pi = i + 1;

        quickSort(str, low, pi - 1);
        quickSort(str, pi + 1, high);
    }
}

int main() {
    std::string str = "quickbrownfox"; //bcfiknooqruwx

    quickSort(str, 0, str.size() - 1);

    std::cout << "String ordenada:\n" << str << "\n";

    return 0;
}