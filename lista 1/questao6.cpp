#include <iostream>
#include <string>
#include <vector>
#include <limits>
using namespace std;

// cadastro hospitalar

struct Pessoa {
	string nome;
	int idade;
	string tipoSanguineo;
	string telefone;
};

class Hospital {
public:
	Hospital() : nPessoas(0) {}

	void adicionarPessoa(const Pessoa& p) {
		pessoas.push_back(p);
		nPessoas++;
	}

	Pessoa* getPessoa(int pos) {
		if (pos < 0 || pos >= nPessoas) {
			cout << "Posição inválida" << endl;
			return nullptr;
		}
		return &pessoas[pos];
	}

	void imprimirPessoa(int pos) {
		if (pos < 0 || pos >= nPessoas) {
			cout << "Posição inválida" << endl;
			return;
		}
		cout << "Nome: " << pessoas[pos].nome << '\n';
		cout << "Idade: " << pessoas[pos].idade << '\n';
		cout << "Tipo Sanguíneo: " << pessoas[pos].tipoSanguineo << '\n';
		cout << "Telefone: " << pessoas[pos].telefone << "\n\n";
	}

	void atualizarTelefone(int pos, const string& tel) {
		if (auto* p = getPessoa(pos)) {
			p->telefone = tel;
		}
	}

	void atualizarTipoSanguineo(int pos, const string& tipo) {
		if (auto* p = getPessoa(pos)) {
			p->tipoSanguineo = tipo;
		}
	}

	void atualizarIdade(int pos, int idade) {
		if (auto* p = getPessoa(pos)) {
			p->idade = idade;
		}
	}

	int nPessoas;

private:
	vector<Pessoa> pessoas;
};

static void limparEntrada() {
	cin.clear();
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

static int getPos() {
	int pos;
	cout << "Digite a posição da pessoa: ";
	cin >> pos;
	limparEntrada();
	cout << '\n';
	return pos;
}

int main() {
	bool sair = false;
	int op;
	Hospital hospital;

	cout << "Sistema de cadastro hospitalar, escolha uma opção:\n";
	cout << "1 - Cadastrar pessoa\n";
	cout << "2 - Listar informações de uma pessoa\n";
	cout << "3 - Atualizar telefone\n";
	cout << "4 - Atualizar tipo sanguíneo\n";
	cout << "5 - Atualizar idade\n";
	cout << "6 - Sair\n";

	while (!sair) {
		cout << "Digite uma opção: ";
		if (!(cin >> op)) {
			limparEntrada();
			cout << "Opção inválida\n\n";
			continue;
		}
		limparEntrada();

		switch (op) {
			case 1: {
				Pessoa p; 
				cout << "Nome: ";
				getline(cin, p.nome);
				cout << "Idade: ";
				cin >> p.idade;
				limparEntrada();
				cout << "Tipo sanguíneo: ";
				getline(cin, p.tipoSanguineo);
				cout << "Telefone: ";
				getline(cin, p.telefone);
				hospital.adicionarPessoa(p);
				cout << "Pessoa cadastrada\n\n";
			} break;

			case 2: {
				int pos = getPos();
				hospital.imprimirPessoa(pos - 1);
			} break;

			case 3: {
				int pos = getPos();
				string tel;
				cout << "Novo telefone: ";
				getline(cin, tel);
				hospital.atualizarTelefone(pos - 1, tel);
			} break;

			case 4: {
				int pos = getPos();
				string tipo;
				cout << "Novo tipo sanguíneo: ";
				getline(cin, tipo);
				hospital.atualizarTipoSanguineo(pos - 1, tipo);
			} break;

			case 5: {
				int pos = getPos();
				int idade;
				cout << "Nova idade: ";
				cin >> idade;
				limparEntrada();
				hospital.atualizarIdade(pos - 1, idade);
			} break;

			case 6:
				sair = true;
				break;

			default:
				cout << "Opção inválida\n\n";
				break;
		}
	}

	return 0;
}

