#include <iostream>
using namespace std;
class complexNumber{
    public:
        double real;
        double imag;
    public:
        complexNumber(double r, double i){
            real = r;
            imag = i;
        }
        friend std::ostream& operator<<(std::ostream& os, const complexNumber& c){
            os << c.real << " + " << c.imag << "i";
            return os;
        }
        complexNumber operator+(complexNumber other){
            return complexNumber(real + other.real, imag + other.imag);
        }
        complexNumber operator-(complexNumber other){
            return complexNumber(real - other.real, imag - other.imag);
        }
        complexNumber operator*(complexNumber other){
            return complexNumber(real * other.real - imag * other.imag, real * other.imag + imag * other.real);
        }
        complexNumber operator/(complexNumber other){
            double denominator = other.real * other.real + other.imag * other.imag;
            return complexNumber((real * other.real + imag * other.imag) / denominator, (imag * other.real - real * other.imag) / denominator);
        }
};

int main(){
    complexNumber c1(1, 2);
    complexNumber c2(3, 4);
    cout << c1 + c2 << endl;
    cout << c1 - c2 << endl;
    cout << c1 * c2 << endl;
    cout << c1 / c2 << endl;
    return 0;
}
