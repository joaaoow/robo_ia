#ifndef MAPA_H
#define MAPA_H

typedef struct {
    int r;
    int c;
} Ponto;

typedef enum {
    LIMPAR,
    MOVER_N,
    MOVER_S,
    MOVER_L,
    MOVER_O,
    FICAR
} Acao;

typedef struct {
    int N;
    int M;
    int T;
    char **g;
    Ponto S;
    int sujeira_total;
} Mapa;

int mapa_ler(Mapa *mapa);
void mapa_free(Mapa *mapa);
void imprimir_mapa(Mapa *mapa, Ponto robo);

int dentro(Mapa *mapa, int r, int c);
int eh_bloqueio(Mapa *mapa, int r, int c);
int eh_sujo(Mapa *mapa, int r, int c);

#endif
