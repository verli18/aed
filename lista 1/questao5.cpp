#include <iostream>
using namespace std;

class circuloCubo{
    public:

    circuloCubo(float L, float R){
        length = L;
        radius = R;
    }
    
    void setLenght( float L){
        length = L;
    }
    void setRadius( float R){
        radius = R;
    }
    void printVolume(){
        cout << "Volume do cubo: " << length * length * length << endl;
    }
    void printComprimentoCirculo(){
        cout << "Comprimento do circulo: " << 2 * 3.14 * radius << endl;
    }
    void printAreaCirculo(){
        cout << "Area do circulo: " << 3.14 * radius * radius << endl;
    }
    void printAreaLateral(){
        cout << "Area lateral do cubo: " << length * length << endl;
    }
    void printAreaTotal(){
        cout << "Area total do cubo: " << 6 * length * length << endl;
    }
    
    private:
        float length;
        float radius;
};    

int main(){
    float L, R;
    cout << "Digite o comprimento do cubo: ";
    cin >> L;
    cout << "Digite o raio do circulo: ";
    cin >> R;
    circuloCubo cubo(L, R);
    cubo.printVolume();
    cubo.printComprimentoCirculo();
    cubo.printAreaCirculo();
    cubo.printAreaLateral();
    cubo.printAreaTotal();
    return 0;
}
