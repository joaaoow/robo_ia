cmake_minimum_required(VERSION 3.10)
project(robo_limpador C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(robo_limpador
        main.c
        agente.c
        mapa.c
        log.c)
