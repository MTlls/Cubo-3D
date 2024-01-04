#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define TELA_LARGURA 25
#define TELA_ALTURA 88
#define ALTURA (int)(10)
#define LARGURA (int)(10)
#define OFFSET_X (int)44
#define OFFSET_Y (int)10
#define _USE_MATH_DEFINES
#define PROFUNDIDADE_BUFFER 100

double *zbuffer, ultimo_z_calculado;

/**
 * Essa struct serve para nao haver recalculo dos senos e cossenos
 */
struct TrigValues {
    double cos_a, sin_a, cos_b, sin_b, cos_c, sin_c;
};

typedef struct TrigValues t_TrigValues;

/**
 * Funcao que libera o buffer, qualquer um que seja
 */
void libera_buffer(void* buffer) {
    free(buffer);
}

/**
 * Função que gira um ponto na malha, retorna a nova cordenada do ponto
 */
int gira_coord(int x, int y, int z, t_TrigValues* cos_senos) {
    double new_x, new_y, new_z;
    double cos_a, sin_a, cos_b, sin_b, cos_c, sin_c;
    int coord = 0;

    // fiz isso para impedir o recálculo dessas funcoes
    cos_a = cos_senos->cos_a;
    sin_a = cos_senos->sin_a;
    cos_b = cos_senos->cos_b;
    sin_b = cos_senos->sin_b;
    cos_c = cos_senos->cos_c;
    sin_c = cos_senos->sin_c;

    /* Tranformacoes matriciais. Complicadas. Definitivamente não é o meu forte.
        Esta é uma matriz de rotação intrínseca (roda ao redor do próprio eixo) (a extrínseca é ao redor dos eixos das coordenadas (x, y, z))
        Valores obtidos com calculadoras */
    new_x = (cos_b * cos_c) * x + (cos_b * sin_c) * y - sin_b * z;
    new_y = ((-cos_a * sin_c) + sin_a * sin_b * cos_c) * x + (cos_a * cos_c + sin_a * sin_b * sin_c) * y + (sin_a * cos_b) * z;
    new_z = (sin_a * sin_c + cos_a * sin_b * cos_c) * x + (-(sin_a * cos_c) + cos_a * sin_b * sin_c) * y + (cos_a * cos_b) * z;

    // esses desvios sempre são somados devido aos pontos do cubo nao estarem coladas no eixo
    new_x = new_x + OFFSET_X;
    // divido por 2 pois a altura eh menor em dimensao que a largura do terminal
    new_y = (new_y / 2) + OFFSET_Y;

    coord = (int)new_x + ((int)new_y * TELA_ALTURA);

    // sera visto novamente no calculo do buffer-z, preferi deixar global
    ultimo_z_calculado = (new_z);

    return coord;
}

/**
 * Funcao que insere o caractere ascii mandado como parametro na malha.
 * aqui eh feita a analise do zbuffer.
 */
void insere_ascii(char* buffer, int coord, char c) {
    if (coord >= 0 && coord < TELA_LARGURA * TELA_ALTURA) {
        // so se coloca o caractere caso ele esteja mais proximo que o valor atual do zbuffer nessa coordenada
        if (ultimo_z_calculado > zbuffer[coord]) {
            buffer[coord] = c;

            // valor do zbuffer atualizado, pois ainda pode receber atualizacoes
            zbuffer[coord] = ultimo_z_calculado;
        }
    }
}

/**
 * Funcao que trasnforma a malha/imagem que sera vista no terminal.
 */
void transforma_malha(char* buffer, t_TrigValues* cos_senos) {
    int coord = 0;
    for (int i = -ALTURA; i < ALTURA; i++) {
        for (int j = -LARGURA; j < LARGURA; j++) {
            /* cada giro de ponto é para um dos caracteres, qual deles
            realmente vai aparecer é decidido pelo z-buffer.
            os tres parametros ditam qual face que esta sendo calculada
            (pode ser altura no lugar de largura, pois eh um cubo).
            */
            coord = gira_coord(j, i, -LARGURA, cos_senos);
            insere_ascii(buffer, coord, '#');

            coord = gira_coord(-LARGURA, i, j, cos_senos);
            insere_ascii(buffer, coord, '?');

            coord = gira_coord(LARGURA, i, j, cos_senos);
            insere_ascii(buffer, coord, '~');

            coord = gira_coord(-j, i, LARGURA, cos_senos);
            insere_ascii(buffer, coord, '@');

            coord = gira_coord(j, -LARGURA, i, cos_senos);
            insere_ascii(buffer, coord, '*');

            coord = gira_coord(j, LARGURA, i, cos_senos);
            insere_ascii(buffer, coord, '.');
        }
    }
}

/**
 * Imprime o buffer
 */
void imprime_buffer(char* buffer) {
    for (int i = 0, j = 0; i < TELA_LARGURA * TELA_ALTURA; i++, j++) {
        if (j % TELA_ALTURA == 0)
            putchar('\n');
        printf("%c", buffer[i]);
    }
}

/**
 * Inicializa o zbuffer
 */
void inicializa_zbuffer() {
    for (int i = 0; i < TELA_LARGURA * TELA_ALTURA; i++) {
        zbuffer[i] = 0;
    }
}

/**
 * Funcao criada para atualizar os valores dos senos e cossenos dos angulos.
 */
t_TrigValues* calcula_sin_cos(t_TrigValues* valores, double a, double b, double c) {
    valores->cos_a = cos(a);
    valores->sin_a = sin(a);
    valores->cos_b = cos(b);
    valores->sin_b = sin(b);
    valores->cos_c = cos(c);
    valores->sin_c = sin(c);

    return valores;
}

int main() {
    char* malha = NULL;
    double alfa = 0;
    double beta = 0;
    double gama = 0;

    t_TrigValues* cos_senos = (t_TrigValues*)malloc(sizeof(t_TrigValues));

    // alocacao dos buffers
    malha = (char*)malloc(sizeof(double) * TELA_LARGURA * TELA_ALTURA);
    zbuffer = (double*)malloc(sizeof(double) * TELA_LARGURA * TELA_ALTURA);

    while (1) {
        // zera a malha
        memset(malha, ' ', TELA_LARGURA * TELA_ALTURA);
        // seta todos os valores do zbuffer para 0
        inicializa_zbuffer();

        // faz a transformacao do cubo dado os angulos (que aumentaram)
        transforma_malha(malha, cos_senos);
        // limpa a tela (queremos animacao!!!!!)
        system("clear || cls");
        
        imprime_buffer(malha);

        // impedindo que os angulos crescam muito (calculamos os sin/cos desses caras!)
        if (gama > M_PI)
            gama == 0;
        if (alfa > M_PI)
            alfa == 0;
        if (beta > M_PI)
            beta == 0;

        // aumenta o grau de rotacao
        gama += .0873;
        beta += .2618;
        alfa += .1745;
        calcula_sin_cos(cos_senos, alfa, beta, gama);
    }

    libera_buffer(malha);
    return 0;
}