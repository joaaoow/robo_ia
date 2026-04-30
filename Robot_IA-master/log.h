#ifndef LOG_H
#define LOG_H

#include "mapa.h"

typedef struct {
    Acao *v;
    int cap;
    int ini;
    int sz;
} Log;

void log_init(Log *log, int cap);
void log_push(Log *log, Acao acao);
void log_free(Log *log);

#endif
