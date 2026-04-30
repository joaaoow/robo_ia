#ifndef AGENTE_H
#define AGENTE_H

#include "mapa.h"

#define MOTIVO_LEN 128

const char *nome_acao(Acao acao);

Acao decide_reflex(Mapa *mapa, Ponto robo, char *motivo);

int aplicar_acao(Mapa *mapa, Ponto *robo, Acao acao, int *limpezas, int *bloqueios);

#endif
