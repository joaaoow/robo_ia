#include <stdio.h>
#include <string.h>
#include "agente.h"

typedef struct {
    Acao acao;
    int dr;
    int dc;
    const char *nome;
    const char *motivo_sujo;
} Direcao;

static const Direcao DIRECOES[] = {
    { MOVER_N, -1,  0, "MOVER_N", "Regra 2: sujeira ao norte" },
    { MOVER_S,  1,  0, "MOVER_S", "Regra 2: sujeira ao sul" },
    { MOVER_L,  0,  1, "MOVER_L", "Regra 2: sujeira ao leste" },
    { MOVER_O,  0, -1, "MOVER_O", "Regra 2: sujeira ao oeste" }
};

static const int TOTAL_DIRECOES = (int)(sizeof(DIRECOES) / sizeof(DIRECOES[0]));

static void registrar_motivo(char *destino, const char *texto) {
    snprintf(destino, MOTIVO_LEN, "%s", texto);
}

static const Direcao *buscar_direcao(Acao acao) {
    for (int i = 0; i < TOTAL_DIRECOES; i++) {
        if (DIRECOES[i].acao == acao)
            return &DIRECOES[i];
    }

    return NULL;
}

static int tentar_sujeira_vizinha(Mapa *mapa, Ponto pos, Acao *acao, char *motivo) {
    for (int i = 0; i < TOTAL_DIRECOES; i++) {
        const Direcao *dir = &DIRECOES[i];

        if (eh_sujo(mapa, pos.r + dir->dr, pos.c + dir->dc)) {
            *acao = dir->acao;
            registrar_motivo(motivo, dir->motivo_sujo);
            return 1;
        }
    }

    return 0;
}

static int tentar_movimento(Mapa *mapa, Ponto pos, Acao acao, const char *motivo_texto,
                            Acao *escolhida, char *motivo) {
    const Direcao *dir = buscar_direcao(acao);

    if (dir == NULL)
        return 0;

    if (eh_bloqueio(mapa, pos.r + dir->dr, pos.c + dir->dc))
        return 0;

    *escolhida = acao;
    registrar_motivo(motivo, motivo_texto);
    return 1;
}

static int tentar_varredura(Mapa *mapa, Ponto pos, Acao *acao, char *motivo) {
    static const Acao linha_par[] = { MOVER_L, MOVER_S, MOVER_O };
    static const Acao linha_impar[] = { MOVER_O, MOVER_S, MOVER_L };

    static const char *motivos_par[] = {
        "Regra 3: varredura em linha par para leste",
        "Regra 3: varredura em linha par para sul",
        "Regra 3: varredura em linha par para oeste"
    };

    static const char *motivos_impar[] = {
        "Regra 3: varredura em linha impar para oeste",
        "Regra 3: varredura em linha impar para sul",
        "Regra 3: varredura em linha impar para leste"
    };

    const Acao *ordem = (pos.r % 2 == 0) ? linha_par : linha_impar;
    const char **motivos = (pos.r % 2 == 0) ? motivos_par : motivos_impar;

    for (int i = 0; i < 3; i++) {
        if (tentar_movimento(mapa, pos, ordem[i], motivos[i], acao, motivo))
            return 1;
    }

    return 0;
}

static int tentar_recuo_vertical(Mapa *mapa, Ponto pos, Acao *acao, char *motivo) {
    if (tentar_movimento(mapa, pos, MOVER_N, "Fallback: subindo para destravar", acao, motivo))
        return 1;

    return tentar_movimento(mapa, pos, MOVER_S, "Fallback: descendo para destravar", acao, motivo);
}

const char *nome_acao(Acao a) {
    if (a == LIMPAR)
        return "LIMPAR";

    if (a == FICAR)
        return "FICAR";

    const Direcao *dir = buscar_direcao(a);
    return (dir != NULL) ? dir->nome : "FICAR";
}

Acao decide_reflex(Mapa *M, Ponto robo, char *motivo) {
    Acao escolhida = FICAR;

    if (M->g[robo.r][robo.c] == '*') {
        registrar_motivo(motivo, "Regra 1: limpando a celula atual");
        return LIMPAR;
    }

    if (tentar_sujeira_vizinha(M, robo, &escolhida, motivo))
        return escolhida;

    if (tentar_varredura(M, robo, &escolhida, motivo))
        return escolhida;

    if (tentar_recuo_vertical(M, robo, &escolhida, motivo))
        return escolhida;

    registrar_motivo(motivo, "Fallback: sem movimento disponivel");
    return FICAR;
}

int aplicar_acao(Mapa *M, Ponto *robo, Acao a, int *limpezas, int *bloqueios) {
    if (a == LIMPAR) {
        if (M->g[robo->r][robo->c] == '*') {
            M->g[robo->r][robo->c] = '.';
            M->sujeira_total--;
            (*limpezas)++;
        }

        return 1;
    }

    const Direcao *dir = buscar_direcao(a);
    if (dir == NULL)
        return 0;

    int destino_r = robo->r + dir->dr;
    int destino_c = robo->c + dir->dc;

    if (eh_bloqueio(M, destino_r, destino_c)) {
        (*bloqueios)++;
        return 0;
    }

    robo->r = destino_r;
    robo->c = destino_c;
    return 1;
}
