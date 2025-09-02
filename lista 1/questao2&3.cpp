#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;
//sistema de cadastro AED

class aluno{
    public:
        aluno( string nome, string matrícula, string curso){
            this->nome = nome;
            this->matrícula = matrícula;
            this->curso = curso;
            faltas = 0;
            nNotas = 0;
            media = 0;
            notas = vector<float>();
        }

        void calcularMedia(){
            media = 0;
            for(int i = 0; i < nNotas; i++){
                media += notas[i];
            }
            media /= nNotas;
        }
        
        void adicionarNota(float nota){
            notas.push_back(nota);
            nNotas++;
            calcularMedia();
        }

        void alterarNota(float nota, int pos){
            if(pos < 0 || pos >= nNotas){
                cout << "Posição de nota inválida!" << endl;
                return;
            }
            notas[pos] = nota;
            calcularMedia();
        }

        void adicionarFalta(){
            faltas++;
        }

        void imprimirNotas(){
            cout << "Notas: " << endl;
            for(int i = 0; i < nNotas; i++){
                cout << i << ": " << notas[i] << endl;
            }
            cout << "Média: " << media << endl;
            cout << "Faltas: " << faltas << endl;
            cout << endl;
        
        }

        string nome;
        string matrícula;
        string curso;
        
    private:

        int nNotas;
        float media;
        int faltas;
        vector<float> notas;
};

class cadastro{
    public:
        cadastro(){
            nAlunos = 0;
        }

        int nAlunos;

        void adicionarAluno(aluno a){
            alunos.push_back(a);
            nAlunos++;
        }

        aluno* getAluno(int pos){
            if(pos < 0 || pos >= nAlunos){
                printf("Posição inválida\n");
                return NULL;
            }
            return &alunos[pos];
        }

        void imprimirAluno(int pos){
            if(pos < 0 || pos >= nAlunos){
                printf("Posição inválida\n");
                return;
            }
            printf("Nome: %s\n", alunos[pos].nome.c_str());
            printf("Matrícula: %s\n", alunos[pos].matrícula.c_str());
            printf("Curso: %s\n", alunos[pos].curso.c_str());
            alunos[pos].imprimirNotas();
        }

    private:
        vector<aluno> alunos;
};

int getPos(){
    int pos;
    cout << "Digite a posição do aluno: ";
    cin >> pos;
    cout << "\n";
    return pos;
}

int main(){
    bool shouldClose = false;
    int op;
    int pos;
    float nota;
    cadastro cadastro;
    

    cout << "Sistema de cadastro, escolha uma opção: \n";
    cout << "1 - Cadastrar aluno\n";
    cout << "2 - Listar informações de um aluno\n";
    cout << "3 - Adicionar nota\n";
    cout << "4 - Adicionar falta\n";
    cout << "5 - Alterar nota\n";
    cout << "6 - Sair\n";

    while(!shouldClose){

        cout << "Digite uma opção: ";
        cin >> op;

        switch(op){
            case 1: 
                {
                    string nome, matricula, curso;
                    cout << "Digite o nome: ";
                    cin >> nome;
                    cout << "Digite a matrícula: ";
                    cin >> matricula;
                    cout << "Digite o curso: ";
                    cin >> curso;
                    aluno novoAluno(nome, matricula, curso);
                    cadastro.adicionarAluno(novoAluno);
                    cout << "Aluno cadastrado\n\n";
                }
                break;
            case 2:
                pos = getPos();
                cadastro.imprimirAluno(pos-1);
                break;
            case 3:
                pos = getPos();
                {
                    aluno* a = cadastro.getAluno(pos-1);
                    if(a){
                        cout << "Digite a nota: ";
                        cin >> nota;
                        a->adicionarNota(nota);
                    }
                }
                break;
            case 4:
                pos = getPos();
                {
                    aluno* a = cadastro.getAluno(pos-1);
                    if(a){
                        a->adicionarFalta();
                    }
                }
                break;
            case 5:
                pos = getPos();
                {
                    aluno* a = cadastro.getAluno(pos-1);
                    if(a){
                        int pos_nota;
                        cout << "Digite a posição da nota a ser alterada: ";
                        cin >> pos_nota;
                        cout << "Digite a nova nota: ";
                        cin >> nota;
                        a->alterarNota(nota, pos_nota);
                    }
                }
                break;       
            case 6:
                shouldClose = true;
                break;
            default:
                cout << "Opção inválida\n\n";
                break;
        }
    }

    return 0;
}
