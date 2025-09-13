#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iomanip>
using namespace std;

class searchAed {
public:
    searchAed() {
        srand(time(NULL));
        populateRandom();
    }
    int vet[256];

    //questão 1
    void populateRandom() {
        for(int i = 0; i < 256; i++){
            vet[i] = rand() % 256;
        }
    }
    
    //questão 2
    void bubbleSort(){
        for(int n = 256; n > 1; n--){
            for (int i = 0; i < n - 1; i++) {
                if (vet[i] > vet[i + 1]) {
                    int temp = vet[i];
                    vet[i] = vet[i + 1];
                    vet[i + 1] = temp;
                }
            }
        }
    }
    
    //questão 3
    int pesqSec(int value, int& nSearches) {
        for(int i = 0; i < 256; i++) {
            nSearches++;
            if(vet[i] == value) {
                return i;
            }
        }
        return -1;
    }

    //questão 4
    int pesqBin(int value, int& nSearches) {
        int left = 0;
        int right = 255;
        while (left <= right) {
            nSearches++;
            int mid = left + (right - left) / 2;
            if (vet[mid] == value) {
                return mid;
            } else if (vet[mid] < value) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        return -1;
    }

    //questão 5
    int pesqSent(int value, int& nSearches){
        int last = vet[255];
        vet[255] = value;
        int i = 0;
        while(vet[i] != value){
            nSearches++;
            i++;
        }
        vet[255] = last;
        if(i < 255 || last == value) {
            return i;
        }
        return -1;
    }
    
    void printVector() {
        for (int i = 0; i < 16; i++) {
            cout << "[" << setw(3) << i*16 << "-" << setw(3) << (i+1)*16 << "]";
            for (int j = 0; j < 16; j++) {
                cout << " " << setw(3) << setfill('0') << vet[i * 16 + j];
            }
            cout << endl;
        }
        cout << endl;
    }
};