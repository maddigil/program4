#ifndef LOGIC_H
#define LOGIC_H

#include "sqlite3.h"
#include "db.h"
#include "config.h"

void fecha_ahora(char *buf, int size);
void log_escribir(const Config *cfg, const char *mensaje);
int leer_entero(const char *prompt, int min, int max);
void leer_cadena(const char *prompt, char *buf, int size);

int logic_registrar_averia(sqlite3 *db, const Config *cfg);
int logic_cambiar_estado_vehiculo(sqlite3 *db, const Config *cfg);
int logic_cambiar_contrasena_usuario(sqlite3 *db);
void logic_mostrar_log(const Config *cfg);

#endif