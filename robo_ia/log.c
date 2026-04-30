#include <stdio.h>
#include <stdlib.h>
#include "log.h"

static int proxima_posicao(Log *log) {
    return (log->ini + log->sz) % log->cap;
}

static void descartar_mais_antigo(Log *log) {
    log->ini = (log->ini + 1) % log->cap;
}

void log_init(Log *log, int cap) {
    log->v = (Acao *)malloc(sizeof(Acao) * (size_t)cap);
    log->cap = cap;
    log->ini = 0;
    log->sz = 0;

    if (log->v == NULL) {
        fprintf(stderr, "Erro: falha ao alocar log\n");
        exit(1);
    }
}

void log_push(Log *log, Acao acao) {
    log->v[proxima_posicao(log)] = acao;

    if (log->sz < log->cap) {
        log->sz++;
    } else {
        descartar_mais_antigo(log);
    }
}

void log_free(Log *log) {
    free(log->v);
    log->v = NULL;
    log->cap = 0;
    log->ini = 0;
    log->sz = 0;
}
