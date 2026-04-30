#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAXN 200
#define MAXM 200
#define LOG_CAP 64
#define MOTIVO_LEN 128

typedef struct { int r, c; } Ponto;

typedef enum {
    LIMPAR, MOVER_N, MOVER_S, MOVER_L, MOVER_O, FICAR
} Acao;

typedef struct {
    Acao v[LOG_CAP];
    int ini, sz;
} Log;

typedef struct {
    int N, M, T;
    char g[MAXN][MAXM + 1]; // +1 para '\0'
    Ponto S;
    int sujeira_total;
} Mapa;

/* ------------------ utilidades ------------------ */

void log_init(Log *L) {
    L->ini = 0;
    L->sz = 0;
}

void log_push(Log *L, Acao a) {
    int pos = (L->ini + L->sz) % LOG_CAP;
    L->v[pos] = a;
    if (L->sz < LOG_CAP) {
        L->sz++;
    } else {
        L->ini = (L->ini + 1) % LOG_CAP;
    }
}

int dentro(Mapa *M, int r, int c) {
    return (r >= 0 && r < M->N && c >= 0 && c < M->M);
}

int eh_bloqueio(Mapa *M, int r, int c) {
    if (!dentro(M, r, c)) return 1;
    return M->g[r][c] == '#';
}

int eh_sujo(Mapa *M, int r, int c) {
    if (!dentro(M, r, c)) return 0;
    return M->g[r][c] == '*';
}

const char *nome_acao(Acao a) {
    switch (a) {
        case LIMPAR: return "LIMPAR";
        case MOVER_N: return "MOVER_N";
        case MOVER_S: return "MOVER_S";
        case MOVER_L: return "MOVER_L";
        case MOVER_O: return "MOVER_O";
        default: return "FICAR";
    }
}

void imprimir_mapa(Mapa *M, Ponto robo) {
    for (int i = 0; i < M->N; i++) {
        for (int j = 0; j < M->M; j++) {
            if (i == robo.r && j == robo.c) printf("R");
            else printf("%c", M->g[i][j]);
        }
        printf("\n");
    }
}

void espera_enter() {
    printf("Pressione ENTER para continuar...");
    getchar();
}

/* ------------------ decisão do agente ------------------ */

Acao decide_reflex(Mapa *M, Ponto robo, char *motivo) {
    int r = robo.r, c = robo.c;

    if (M->g[r][c] == '*') {
        strcpy(motivo, "Regra 1: celula atual suja -> limpar");
        return LIMPAR;
    }

    if (eh_sujo(M, r - 1, c)) {
        strcpy(motivo, "Regra 2: vizinho sujo ao norte");
        return MOVER_N;
    }
    if (eh_sujo(M, r + 1, c)) {
        strcpy(motivo, "Regra 2: vizinho sujo ao sul");
        return MOVER_S;
    }
    if (eh_sujo(M, r, c + 1)) {
        strcpy(motivo, "Regra 2: vizinho sujo ao leste");
        return MOVER_L;
    }
    if (eh_sujo(M, r, c - 1)) {
        strcpy(motivo, "Regra 2: vizinho sujo ao oeste");
        return MOVER_O;
    }

    if (c % 2 == 0) {
        if (!eh_bloqueio(M, r, c + 1)) {
            strcpy(motivo, "Regra 3: zig-zag coluna par -> leste");
            return MOVER_L;
        }
        if (!eh_bloqueio(M, r + 1, c)) {
            strcpy(motivo, "Regra 3: zig-zag coluna par -> sul");
            return MOVER_S;
        }
        if (!eh_bloqueio(M, r, c - 1)) {
            strcpy(motivo, "Regra 3: zig-zag coluna par -> oeste");
            return MOVER_O;
        }
    } else {
        if (!eh_bloqueio(M, r, c - 1)) {
            strcpy(motivo, "Regra 3: zig-zag coluna impar -> oeste");
            return MOVER_O;
        }
        if (!eh_bloqueio(M, r + 1, c)) {
            strcpy(motivo, "Regra 3: zig-zag coluna impar -> sul");
            return MOVER_S;
        }
        if (!eh_bloqueio(M, r, c + 1)) {
            strcpy(motivo, "Regra 3: zig-zag coluna impar -> leste");
            return MOVER_L;
        }
    }

    if (!eh_bloqueio(M, r - 1, c)) {
        strcpy(motivo, "Fallback: mover norte");
        return MOVER_N;
    }
    if (!eh_bloqueio(M, r + 1, c)) {
        strcpy(motivo, "Fallback: mover sul");
        return MOVER_S;
    }

    strcpy(motivo, "Fallback final: ficar parado");
    return FICAR;
}

/* ------------------ execução da ação ------------------ */

int aplicar_acao(Mapa *M, Ponto *robo, Acao a, int *limpezas, int *bloqueios) {
    int r = robo->r, c = robo->c;
    int nr = r, nc = c;

    if (a == LIMPAR) {
        if (M->g[r][c] == '*') {
            M->g[r][c] = '.';
            M->sujeira_total--;
            (*limpezas)++;
        }
        return 1;
    }

    if (a == MOVER_N) nr--;
    if (a == MOVER_S) nr++;
    if (a == MOVER_L) nc++;
    if (a == MOVER_O) nc--;

    if (eh_bloqueio(M, nr, nc)) {
        (*bloqueios)++;
        return 0;
    }

    robo->r = nr;
    robo->c = nc;
    return 1;
}

/* ------------------ leitura do mapa ------------------ */

int linha_valida(char *s, int M) {
    if ((int)strlen(s) != M) return 0;
    for (int i = 0; i < M; i++) {
        if (!(s[i] == '.' || s[i] == '*' || s[i] == '#' || s[i] == 'S'))
            return 0;
    }
    return 1;
}

/* ------------------ main ------------------ */

int main() {

    Mapa M;
    Log log;
    Ponto robo;

    int limpezas = 0;
    int bloqueios = 0;
    int passo = 0;
    int passo_a_passo = 0;

    char motivo[MOTIVO_LEN];

    printf("Digite N M T:\n");
    scanf("%d %d %d", &M.N, &M.M, &M.T);

    printf("Digite o mapa:\n");

    M.sujeira_total = 0;

    for (int i = 0; i < M.N; i++) {

        scanf("%s", M.g[i]);

        if (!linha_valida(M.g[i], M.M)) {
            printf("Linha invalida\n");
            return 1;
        }

        for (int j = 0; j < M.M; j++) {

            if (M.g[i][j] == 'S') {
                M.S.r = i;
                M.S.c = j;
                M.g[i][j] = '.';
            }

            if (M.g[i][j] == '*')
                M.sujeira_total++;
        }
    }

    robo = M.S;

    printf("Modo passo-a-passo? (1=sim 0=nao): ");
    scanf("%d", &passo_a_passo);
    getchar();

    log_init(&log);

    clock_t inicio = clock();

    while (passo < M.T && M.sujeira_total > 0) {

        Acao a = decide_reflex(&M, robo, motivo);

        int ok = aplicar_acao(&M, &robo, a, &limpezas, &bloqueios);

        log_push(&log, a);

        if (passo_a_passo) {

            printf("\nPasso %d\n", passo);
            printf("Acao: %s (%s) -> %s\n",
                   nome_acao(a),
                   motivo,
                   ok ? "ok" : "bloqueado");

            imprimir_mapa(&M, robo);

            printf("Sujeira restante: %d | Limpezas: %d | Bloqueios: %d\n",
                   M.sujeira_total, limpezas, bloqueios);

            espera_enter();
        }

        passo++;
    }

    clock_t fim = clock();

    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;

    int sujeira_inicial = limpezas + M.sujeira_total;

    double porcentagem = 0;
    if (sujeira_inicial > 0)
        porcentagem = (100.0 * limpezas) / sujeira_inicial;

    printf("\n===== RESULTADOS =====\n");
    printf("Passos executados: %d\n", passo);
    printf("Limpezas: %d\n", limpezas);
    printf("Bloqueios: %d\n", bloqueios);
    printf("Sujeira restante: %d\n", M.sujeira_total);
    printf("Percentual limpo: %.2f%%\n", porcentagem);
    printf("Tempo CPU: %.6f s\n", tempo);

    printf("\nMapa final:\n");
    imprimir_mapa(&M, robo);

    printf("\nPosicao final do robo: (%d,%d)\n", robo.r, robo.c);

    return 0;
}//
// Created by luan on 05/03/26.
//