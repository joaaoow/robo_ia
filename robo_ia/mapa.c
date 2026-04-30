#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mapa.h"

static int caractere_do_mapa(char c) {
    return c == '.' || c == '*' || c == '#' || c == 'S' || c == '!';
}

static int linha_valida(const char *linha, int colunas) {
    if ((int)strlen(linha) != colunas)
        return 0;

    for (int i = 0; i < colunas; i++) {
        if (!caractere_do_mapa(linha[i]))
            return 0;
    }

    return 1;
}

static void zerar_mapa(Mapa *mapa) {
    mapa->N = 0;
    mapa->M = 0;
    mapa->T = 0;
    mapa->g = NULL;
    mapa->S.r = 0;
    mapa->S.c = 0;
    mapa->sujeira_total = 0;
}

static int alocar_grade(Mapa *mapa) {
    mapa->g = (char **)calloc((size_t)mapa->N, sizeof(char *));
    if (mapa->g == NULL) {
        fprintf(stderr, "Erro: falha ao alocar grade\n");
        return 0;
    }

    for (int i = 0; i < mapa->N; i++) {
        mapa->g[i] = (char *)malloc((size_t)mapa->M + 1);
        if (mapa->g[i] == NULL) {
            fprintf(stderr, "Erro: falha ao alocar linha %d\n", i);
            mapa_free(mapa);
            return 0;
        }
    }

    return 1;
}

static int primeira_celula_livre(Mapa *mapa, Ponto *saida) {
    for (int i = 0; i < mapa->N; i++) {
        for (int j = 0; j < mapa->M; j++) {
            if (mapa->g[i][j] == '.') {
                saida->r = i;
                saida->c = j;
                return 1;
            }
        }
    }

    return 0;
}

static void definir_inicio_padrao(Mapa *mapa) {
    if (primeira_celula_livre(mapa, &mapa->S)) {
        printf("Aviso: 'S' nao encontrado. Usando primeira celula livre (%d,%d).\n",
               mapa->S.r, mapa->S.c);
        return;
    }

    mapa->S.r = 0;
    mapa->S.c = 0;
    printf("Aviso: nenhuma celula livre encontrada. Usando (0,0).\n");
}

static void processar_celula(Mapa *mapa, int r, int c, int *inicio_encontrado) {
    if (mapa->g[r][c] == 'S') {
        mapa->S.r = r;
        mapa->S.c = c;
        mapa->g[r][c] = '.';
        *inicio_encontrado = 1;
        return;
    }

    if (mapa->g[r][c] == '*')
        mapa->sujeira_total++;
}

int mapa_ler(Mapa *mapa) {
    zerar_mapa(mapa);

    printf("Digite N M T:\n");
    if (scanf("%d %d %d", &mapa->N, &mapa->M, &mapa->T) != 3) {
        fprintf(stderr, "Erro: entrada N M T invalida\n");
        return 0;
    }

    if (mapa->N <= 0 || mapa->M <= 0 || mapa->T < 0) {
        fprintf(stderr, "Erro: dimensoes ou limite de passos invalidos\n");
        return 0;
    }

    if (!alocar_grade(mapa))
        return 0;

    printf("Digite o mapa (%d linhas, %d colunas):\n", mapa->N, mapa->M);

    int inicio_encontrado = 0;

    for (int i = 0; i < mapa->N; i++) {
        if (scanf("%s", mapa->g[i]) != 1) {
            fprintf(stderr, "Erro: falha ao ler linha %d\n", i);
            mapa_free(mapa);
            return 0;
        }

        if (!linha_valida(mapa->g[i], mapa->M)) {
            fprintf(stderr, "Erro: linha %d invalida\n", i);
            mapa_free(mapa);
            return 0;
        }

        for (int j = 0; j < mapa->M; j++)
            processar_celula(mapa, i, j, &inicio_encontrado);
    }

    if (!inicio_encontrado)
        definir_inicio_padrao(mapa);

    return 1;
}

void mapa_free(Mapa *mapa) {
    if (mapa->g == NULL)
        return;

    for (int i = 0; i < mapa->N; i++)
        free(mapa->g[i]);

    free(mapa->g);
    mapa->g = NULL;
}

void imprimir_mapa(Mapa *mapa, Ponto robo) {
    for (int i = 0; i < mapa->N; i++) {
        for (int j = 0; j < mapa->M; j++) {
            putchar((i == robo.r && j == robo.c) ? 'R' : mapa->g[i][j]);
        }
        putchar('\n');
    }
}

int dentro(Mapa *mapa, int r, int c) {
    return r >= 0 && r < mapa->N && c >= 0 && c < mapa->M;
}

int eh_bloqueio(Mapa *mapa, int r, int c) {
    return !dentro(mapa, r, c) || mapa->g[r][c] == '#';
}

int eh_sujo(Mapa *mapa, int r, int c) {
    return dentro(mapa, r, c) && mapa->g[r][c] == '*';
}
