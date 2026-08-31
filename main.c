#include <stdio.h>

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

int main() {

    return 0;
}