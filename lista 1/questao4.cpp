#include "q4c.hpp"
#include <iostream>

std::string gerenciadorEstacionamento::adicionarCarro( entidadeCarro carro){
    //Seria bom implementar checks de string para a placa, telefone e marca, atualmente é fácil cometer um overflow ( inseguro e permite vulnerabilidades.)
    carrosEstacionados[carro.placa] = carro;
    nCarros++;
    return carro.placa;
}

int gerenciadorEstacionamento::removerCarro(std::string placa){
    carrosEstacionados.erase(placa);
    nCarros--;
    return 0; // Return success
}

entidadeCarro gerenciadorEstacionamento::getCar(std::string placa){
    return carrosEstacionados[placa];
}

int main(){
    gerenciadorEstacionamento gerenciador;
    
    entidadeCarro carro1 = {"ABCD1234", "+61979544475", {244, 200, 122}, "Ford"};
    std::string placa = gerenciador.adicionarCarro( carro1);

    std::cout << gerenciador.getCar(placa).marca << std::endl << gerenciador.getCar(placa).placa << std::endl << gerenciador.getCar(placa).telefone;
    return 0;
}