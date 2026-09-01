#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int calcular_pixel(double c_real, double c_imag, int max_iteracoes) {

    double z_real = 0.0;
    double z_imag = 0.0;

    int iteracao = 0;

    while (iteracao < max_iteracoes && z_real * z_real + z_imag * z_imag <= 4.0) {

        double novo_real = z_real * z_real - z_imag * z_imag + c_real;
        double novo_imag = 2.0 * z_real * z_imag + c_imag;

        z_real = novo_real;
        z_imag = novo_imag;

        iteracao++;
    }
    return iteracao;
}

void calcular_serial(int largura, int altura, int max_iteracoes, int *imagem) {

    for (int linha = 0; linha < altura; linha++) {

        double c_imag = -1.5 + (3.0 * linha) / (altura - 1);

        for (int coluna = 0; coluna < largura; coluna++) {

            double c_real = -2.0 + (3.0 * coluna) / (largura - 1);

            int iteracoes = calcular_pixel(c_real, c_imag, max_iteracoes);

            int intensidade = (iteracoes * 255) / max_iteracoes;

            imagem[linha * largura + coluna] = intensidade;
        }
    }
}

int *criar_imagem(int largura, int altura) {

    int *imagem = malloc(largura * altura * sizeof(int));

    if (imagem == NULL) {
        return NULL;
    }

    return imagem;
}

int converter_inteiro(char *texto, int *valor) {

    char *fim;
    long numero = strtol(texto, &fim, 10);

    if (*texto == '\0' || *fim != '\0' ||
        numero < INT_MIN || numero > INT_MAX) {
        return 0;
    }

    *valor = (int)numero;
    return 1;
}

int main(int argc, char *argv[]) {

    if (argc != 5) {
        fprintf(stderr, "Uso: ./mandelbrot largura altura max_iteracoes num_threads\n");
        return 1;
    }

    int largura;
    int altura;
    int max_iteracoes;
    int num_threads;

    if (!converter_inteiro(argv[1], &largura) || !converter_inteiro(argv[2], &altura) || !converter_inteiro(argv[3], &max_iteracoes) || !converter_inteiro(argv[4], &num_threads)) {

        fprintf(stderr, "Erro: argumentos invalidos.\n");
        return 1;
    }

    if (largura < 2 || altura < 2 ||
        max_iteracoes <= 0 || num_threads <= 0) {

        fprintf(stderr, "Erro: valores invalidos.\n");
        return 1;
    }
    int *imagem = criar_imagem(largura, altura);

    if (imagem == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        return 1;
    }

    calcular_serial(largura, altura, max_iteracoes, imagem);

    free(imagem);

    return 0;
}