CC     = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = robo_limpador
OBJ    = main.o agente.o mapa.o log.o

# Regra padrão: compila o executável
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compila cada .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Remove objetos e executável
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: clean
