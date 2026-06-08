# Makefile - Euskokar Fase 2
# Estructura:
# fase2/
# ├── servidor/
# ├── cliente/
# ├── comun/
# ├── datos/
# └── Makefile

CC  = gcc
CXX = g++

CFLAGS   = -Wall -Wextra -g -pthread -Iservidor -Icomun
CXXFLAGS = -Wall -Wextra -g -std=c++17 -Icliente -Icomun

SRV_SRCS = servidor/main.c \
           servidor/server.c \
           servidor/db.c \
           servidor/config.c \
           servidor/logic.c \
           servidor/admin.c \
           servidor/sqlite3.c

CLI_SRCS = cliente/main_cliente.cpp \
           cliente/cliente.cpp \
           cliente/menu_usuario.cpp \
           cliente/ascii_mapa.cpp \
           cliente/cache_manager.cpp

SRV_OBJS = $(SRV_SRCS:.c=.o)
CLI_OBJS = $(CLI_SRCS:.cpp=.o)

.PHONY: all servidor.out cliente.out clean help

all: servidor.out cliente.out

servidor.out: $(SRV_OBJS)
	$(CC) $(CFLAGS) -o servidor.out $(SRV_OBJS) -ldl -lm

cliente.out: $(CLI_OBJS)
	$(CXX) $(CXXFLAGS) -o cliente.out $(CLI_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f servidor/*.o cliente/*.o servidor.out cliente.out

help:
	@echo "Comandos disponibles:"
	@echo "  make           -> compila servidor y cliente"
	@echo "  make servidor.out -> solo el servidor"
	@echo "  make cliente.out  -> solo el cliente"
	@echo "  make clean     -> limpia ejecutables y objetos"
	@echo ""
	@echo "Para ejecutar:"
	@echo "  Terminal 1: ./servidor.out"
	@echo "  Terminal 2: ./cliente.out"
