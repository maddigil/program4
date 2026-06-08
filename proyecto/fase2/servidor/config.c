#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int config_cargar(const char *ruta, Config *cfg) {
    FILE *f = fopen(ruta, "r");
    if (!f) {
        printf("Error: No se puede abrir %s\n", ruta);
        return 0;
    }

    cfg->servidor_puerto = PUERTO_DEFAULT;
    cfg->servidor_ip     = NULL;

    char linea[512];
    while (fgets(linea, sizeof(linea), f)) {
        if (linea[0] == '#' || linea[0] == '\n' || linea[0] == '\r')
            continue;

        linea[strcspn(linea, "\r\n")] = '\0';

        char *igual = strchr(linea, '=');
        if (!igual) continue;

        *igual = '\0';
        char *nombre = linea;
        char *valor  = igual + 1;

        while (*nombre == ' ') nombre++;
        while (*valor  == ' ') valor++;

        int len = (int)strlen(valor);
        if (len >= 2 && valor[0] == '"' && valor[len - 1] == '"') {
            valor[len - 1] = '\0';
            valor++;
        }

        char **destino = NULL;

        if      (strcmp(nombre, "admin_usuario")  == 0) destino = &cfg->admin_usuario;
        else if (strcmp(nombre, "admin_clave")    == 0) destino = &cfg->admin_clave;
        else if (strcmp(nombre, "db_path")        == 0) destino = &cfg->db_path;
        else if (strcmp(nombre, "estaciones_csv") == 0) destino = &cfg->estaciones_csv;
        else if (strcmp(nombre, "usuarios_csv")   == 0) destino = &cfg->usuarios_csv;
        else if (strcmp(nombre, "vehiculos_csv")  == 0) destino = &cfg->vehiculos_csv;
        else if (strcmp(nombre, "log_path")       == 0) destino = &cfg->log_path;
        else if (strcmp(nombre, "servidor_ip")    == 0) destino = &cfg->servidor_ip;

        if (destino) {
            free(*destino);
            *destino = malloc(strlen(valor) + 1);
            if (*destino) strcpy(*destino, valor);
            continue;
        }

        if (strcmp(nombre, "servidor_puerto") == 0) {
            cfg->servidor_puerto = atoi(valor);
        }
    }

    fclose(f);
    return 1;
}

void config_mostrar(const Config *cfg) {
    printf("=== CONFIGURACION CARGADA ===\n");
    printf("DB             : %s\n", cfg->db_path       ? cfg->db_path       : "(no definido)");
    printf("CSV Estaciones : %s\n", cfg->estaciones_csv ? cfg->estaciones_csv : "(no definido)");
    printf("CSV Usuarios   : %s\n", cfg->usuarios_csv   ? cfg->usuarios_csv   : "(no definido)");
    printf("CSV Vehiculos  : %s\n", cfg->vehiculos_csv  ? cfg->vehiculos_csv  : "(no definido)");
    printf("Log            : %s\n", cfg->log_path       ? cfg->log_path       : "(no definido)");
    printf("Servidor IP    : %s\n", cfg->servidor_ip    ? cfg->servidor_ip    : "127.0.0.1");
    printf("Servidor Puerto: %d\n", cfg->servidor_puerto);
    printf("=============================\n");
}

void config_liberar(Config *cfg) {
    free(cfg->admin_usuario);  cfg->admin_usuario  = NULL;
    free(cfg->admin_clave);    cfg->admin_clave    = NULL;
    free(cfg->db_path);        cfg->db_path        = NULL;
    free(cfg->estaciones_csv); cfg->estaciones_csv = NULL;
    free(cfg->usuarios_csv);   cfg->usuarios_csv   = NULL;
    free(cfg->vehiculos_csv);  cfg->vehiculos_csv  = NULL;
    free(cfg->log_path);       cfg->log_path       = NULL;
    free(cfg->servidor_ip);    cfg->servidor_ip    = NULL;
}
