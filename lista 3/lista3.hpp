#include <cstdlib>
#include <ctime>

class search {
public:
    search() {
        srand(time(NULL));
        populateRandom();
    }
    int vector[256];

    void populateRandom() {
        for(int i = 0; i < 256; i++){
            vector[i] = rand() % 256;
        }
    }
    
    int linearSearch(int value) {
        for(int i = 0; i < 256; i++) {
            if(vector[i] == value) {
                return i;
            }
        }
        return -1;
    }
};