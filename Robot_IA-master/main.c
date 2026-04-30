#include <stdio.h>
#include <time.h>
#include "mapa.h"
#include "agente.h"
#include "log.h"

#define LOG_CAP 64

typedef struct {
    int passos;
    int limpezas;
    int bloqueios;
    int risco;
} Estatisticas;

static void pausar_execucao(void) {
    printf("Pressione ENTER para continuar...");
    getchar();
}

static int ler_opcao(const char *mensagem) {
    int valor = 0;

    printf("%s", mensagem);
    scanf("%d", &valor);
    getchar();

    return valor;
}

static int acao_movimenta_robo(Acao acao) {
    return acao == MOVER_N || acao == MOVER_S || acao == MOVER_L || acao == MOVER_O;
}

static int entrou_em_risco(Mapa *mapa, Ponto robo, Acao acao, int acao_ok) {
    return acao_ok && acao_movimenta_robo(acao) && mapa->g[robo.r][robo.c] == '!';
}

static void mostrar_passo(Mapa *mapa, Ponto robo, int numero_passo, Acao acao,
                          const char *motivo, int acao_ok, Estatisticas stats) {
    printf("\nPasso %d\n", numero_passo);
    printf("Acao: %s\n", nome_acao(acao));
    printf("Regra: %s\n", motivo);

    if (!acao_ok)
        printf("(movimento bloqueado)\n");

    if (entrou_em_risco(mapa, robo, acao, acao_ok))
        printf("(penalidade de risco: +1 passo extra)\n");

    printf("\nMapa:\n");
    imprimir_mapa(mapa, robo);

    printf("Sujeira restante: %d | Limpezas: %d | Bloqueios: %d\n",
           mapa->sujeira_total, stats.limpezas, stats.bloqueios);

    pausar_execucao();
}

static void imprimir_resultado(Mapa *mapa, Estatisticas stats, double tempo_cpu) {
    int sujeira_inicial = stats.limpezas + mapa->sujeira_total;
    double percentual = (sujeira_inicial > 0)
                        ? (100.0 * stats.limpezas) / sujeira_inicial
                        : 100.0;

    printf("\n===== RESULTADOS =====\n");
    printf("Passos executados : %d\n", stats.passos);
    printf("Limpezas          : %d\n", stats.limpezas);
    printf("Bloqueios         : %d\n", stats.bloqueios);
    printf("Sujeira inicial   : %d\n", sujeira_inicial);
    printf("Sujeira restante  : %d\n", mapa->sujeira_total);
    printf("Percentual limpo  : %.2f%%\n", percentual);
    printf("Entradas em risco : %d\n", stats.risco);
    printf("Tempo de CPU      : %.6f s\n", tempo_cpu);
}

static void talvez_mostrar_mapa_final(Mapa *mapa, Ponto robo) {
    if (!ler_opcao("\nDeseja ver o mapa final? (1=sim 0=nao): "))
        return;

    printf("\nMapa final:\n");
    imprimir_mapa(mapa, robo);
    printf("Posicao final do robo: (%d, %d)\n", robo.r, robo.c);
}

int main(void) {
    Mapa mapa;
    Log historico;
    Ponto robo;
    Estatisticas stats = { 0, 0, 0, 0 };
    char motivo[MOTIVO_LEN];

    if (!mapa_ler(&mapa))
        return 1;

    robo = mapa.S;

    int passo_a_passo = ler_opcao("Modo passo-a-passo? (1=sim 0=nao): ");

    log_init(&historico, LOG_CAP);
    clock_t inicio = clock();

    while (stats.passos < mapa.T && mapa.sujeira_total > 0) {
        Acao acao = decide_reflex(&mapa, robo, motivo);
        int acao_ok = aplicar_acao(&mapa, &robo, acao, &stats.limpezas, &stats.bloqueios);

        log_push(&historico, acao);

        if (passo_a_passo) {
            mostrar_passo(&mapa, robo, stats.passos + 1, acao, motivo, acao_ok, stats);
        }

        if (entrou_em_risco(&mapa, robo, acao, acao_ok)) {
            stats.risco++;
            stats.passos++;
        }

        stats.passos++;
    }

    clock_t fim = clock();
    double tempo_cpu = (double)(fim - inicio) / CLOCKS_PER_SEC;

    imprimir_resultado(&mapa, stats, tempo_cpu);
    talvez_mostrar_mapa_final(&mapa, robo);

    log_free(&historico);
    mapa_free(&mapa);

    return 0;
}
