#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <omp.h>
#include <pthread.h>
typedef struct {
    int inicio;
    int fim;
    int largura;
    int altura;
    int max_iteracoes;
    int *imagem;
} DadosThread;
double tempo_atual() {
    struct timespec tempo;

    clock_gettime(CLOCK_MONOTONIC, &tempo);

    return tempo.tv_sec + tempo.tv_nsec / 1000000000.0;
}

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

void calcular_openmp(int largura, int altura, int max_iteracoes, int num_threads, int *imagem) {

    #pragma omp parallel for num_threads(num_threads)

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

    if (*texto == '\0' || *fim != '\0' || numero < INT_MIN || numero > INT_MAX) {
        return 0;
    }

    *valor = (int)numero;
    return 1;
}

int salvar_imagem(char *nome_arquivo, int *imagem, int largura, int altura) {

    FILE *arquivo = fopen(nome_arquivo, "w");

    if (arquivo == NULL) {
        return 0;
    }

    for (int linha = 0; linha < altura; linha++) {
        for (int coluna = 0; coluna < largura; coluna++) {

            fprintf(arquivo, "%d ", imagem[linha * largura + coluna]);
        }

        fprintf(arquivo, "\n");
    }

    fclose(arquivo);

    return 1;
}
void *calcular_bloco(void *arg) {

    DadosThread *dados = (DadosThread *)arg;

    for (int linha = dados->inicio; linha < dados->fim; linha++) {

        double c_imag = -1.5 + (3.0 * linha) / (dados->altura - 1);

        for (int coluna = 0; coluna < dados->largura; coluna++) {

            double c_real = -2.0 + (3.0 * coluna) / (dados->largura - 1);

            int iteracoes = calcular_pixel(
                c_real,
                c_imag,
                dados->max_iteracoes
            );

            int intensidade =
                (iteracoes * 255) / dados->max_iteracoes;

            dados->imagem[linha * dados->largura + coluna] = intensidade;
        }
    }

    return NULL;
}

int calcular_pthreads1(int largura, int altura, int max_iteracoes, int num_threads, int *imagem) {

    pthread_t threads[num_threads];
    DadosThread dados[num_threads];

    int linhas_por_thread = altura / num_threads;

    for (int i = 0; i < num_threads; i++) {

        dados[i].inicio = i * linhas_por_thread;
        dados[i].fim = (i + 1) * linhas_por_thread;
        dados[i].largura = largura;
        dados[i].altura = altura;
        dados[i].max_iteracoes = max_iteracoes;
        dados[i].imagem = imagem;

        if (i == num_threads - 1) {
            dados[i].fim = altura;
        }

        if (pthread_create(&threads[i], NULL, calcular_bloco, &dados[i]) != 0) {

            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }

            return 0;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

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

    if (largura < 2 || altura < 2 || max_iteracoes <= 0 || num_threads <= 0) {

        fprintf(stderr, "Erro: valores invalidos.\n");
        return 1;
    }
    int *imagem = criar_imagem(largura, altura);

    if (imagem == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        return 1;
    }

    double inicio = tempo_atual();
    calcular_serial(largura, altura, max_iteracoes, imagem);
    double fim = tempo_atual();
    double tempo_serial = fim - inicio;

    if (!salvar_imagem("mandelbrot_masj_serial.pgm", imagem, largura, altura)) {

        fprintf(stderr, "Erro ao criar arquivo de saida.\n");
        free(imagem);
        return 1;
    }

    FILE *arquivo_tempo = fopen("times.txt", "w");

    if (arquivo_tempo == NULL) {
        fprintf(stderr, "Erro ao criar times.txt.\n");
        free(imagem);
        return 1;
    }

    fprintf(arquivo_tempo, "Serial: %f\n", tempo_serial);

    inicio = tempo_atual();

    calcular_openmp(largura, altura, max_iteracoes, num_threads, imagem);

    fim = tempo_atual();

    double tempo_openmp = fim - inicio;

    if (!salvar_imagem("mandelbrot_masj_openmp.pgm", imagem, largura, altura)) {

        fprintf(stderr, "Erro ao criar arquivo de saida.\n");
        fclose(arquivo_tempo);
        free(imagem);
        return 1;
    }
    fprintf(arquivo_tempo, "OpenMP: %f\n", tempo_openmp);

    inicio = tempo_atual();

    if (!calcular_pthreads1(largura, altura, max_iteracoes, num_threads, imagem)) {

        fprintf(stderr, "Erro ao criar thread.\n");
        fclose(arquivo_tempo);
        free(imagem);
        return 1;
    }

    fim = tempo_atual();

    double tempo_pthreads1 = fim - inicio;

    if (!salvar_imagem("mandelbrot_masj_pthreads1.pgm",
                    imagem, largura, altura)) {

        fprintf(stderr, "Erro ao criar arquivo de saida.\n");
        fclose(arquivo_tempo);
        free(imagem);
        return 1;
    }

    fprintf(arquivo_tempo, "Pthreads1: %f\n", tempo_pthreads1);

    fclose(arquivo_tempo);

    free(imagem);

    return 0;
}