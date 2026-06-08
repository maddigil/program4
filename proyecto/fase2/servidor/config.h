#ifndef CONFIG_H
#define CONFIG_H

#include "protocolo.h"   

#define MAX_RUTA  256
#define MAX_VALOR 128

typedef struct {
    char *admin_usuario;
    char *admin_clave;
    char *db_path;
    char *estaciones_csv;
    char *usuarios_csv;
    char *vehiculos_csv;
    char *log_path;
    //IP que escucha el servidor
    char *servidor_ip;
    //puerto  
    int   servidor_puerto;  
} Config;

int  config_cargar(const char *ruta, Config *cfg);

void config_mostrar(const Config *cfg);

void config_liberar(Config *cfg);

#endif