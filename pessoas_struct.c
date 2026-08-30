#include <stdio.h>

typedef struct pessoa {
    char nome[20];
    int idade;
    char cpf[12];
    char rg[20];
    char telefone[20];
} Pessoa;

void lerPessoa(Pessoa *p) {
    printf("Digite o nome: ");
    scanf("%s", p->nome);

    printf("Digite a idade: ");
    scanf("%d", &p->idade);

    printf("Digite o CPF: ");
    scanf("%s", p->cpf);

    printf("Digite o RG: ");
    scanf("%s", p->rg);

    printf("Digite o telefone: ");
    scanf("%s", p->telefone);
}

void imprimirPessoa(Pessoa *p) {
    printf("Nome: %s\n", p->nome);
    printf("Idade: %d\n", p->idade);
    printf("CPF: %s\n", p->cpf);
    printf("RG: %s\n", p->rg);
    printf("Telefone: %s\n", p->telefone);
}

int main(void) {
    Pessoa p;

    lerPessoa(&p);
    imprimirPessoa(&p);

    return 0;
}