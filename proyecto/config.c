#include "config.h"
#include <stdio.h>
#include <string.h>

int config_cargar(const char *ruta, Config *cfg){
    FILE *f = fopen(ruta, "r");
    if(!f){
        printf("Error: No se puede abrir %s\n", ruta);
        return 0;
    }

    char linea[512];
    while(fgets(linea, sizeof(linea), f)){
        // Saltar comentarios y líneas vacías
        if(linea[0] == '#' || linea[0] == '\0'){
            continue;
        }

        // Quitar salto de línea
        linea[strcspn(linea, "\r\n")] = '\0';

        // Buscar nombre=valor
        char *igual = strchr(linea, '=');
        if(!igual) continue;

        *igual = '\0';  // Separar nombre
        char *nombre = linea;
        char *valor = igual + 1;

        // Quitar espacios alrededor
        while(*nombre == ' ') nombre++;
        while(*valor == ' ') valor++;

        // Quitar comillas si las hay
        int len = strlen(valor);
        if(valor[0] == '"' && valor[len-1] == '"'){
            valor[len-1] = '\0';
            valor++;
        }

        // Asignar valores según el nombre
        if(strcmp(nombre, "admin_usuario") == 0){
            strncpy(cfg->admin_usuario, valor, MAX_VALOR-1);
            cfg->admin_usuario[MAX_VALOR-1] = '\0';
        }
        else if(strcmp(nombre, "admin_clave") == 0){
            strncpy(cfg->admin_clave, valor, MAX_VALOR-1);
            cfg->admin_clave[MAX_VALOR-1] = '\0';
        }
        else if(strcmp(nombre, "db_path") == 0){
            strncpy(cfg->db_path, valor, MAX_RUTA-1);
            cfg->db_path[MAX_RUTA-1] = '\0';
        }
        else if(strcmp(nombre, "estaciones_csv") == 0){
            strncpy(cfg->estaciones_csv, valor, MAX_RUTA-1);
            cfg->estaciones_csv[MAX_RUTA-1] = '\0';
        }
        else if(strcmp(nombre, "usuarios_csv") == 0){
            strncpy(cfg->usuarios_csv, valor, MAX_RUTA-1);
            cfg->usuarios_csv[MAX_RUTA-1] = '\0';
        }
        else if(strcmp(nombre, "vehiculos_csv") == 0){
            strncpy(cfg->vehiculos_csv, valor, MAX_RUTA-1);
            cfg->vehiculos_csv[MAX_RUTA-1] = '\0';
        }
        else if(strcmp(nombre, "log_path") == 0){
            strncpy(cfg->log_path, valor, MAX_RUTA-1);
            cfg->log_path[MAX_RUTA-1] = '\0';
        }
    }

    fclose(f);
    return 1;
}

void config_mostrar(const Config *cfg){
    printf("=== CONFIGURACION CARGADA ===\n");
    printf("Admin: %s / %s\n", cfg->admin_usuario, cfg->admin_clave);
    printf("DB: %s\n", cfg->db_path);
    printf("CSV Estaciones: %s\n", cfg->estaciones_csv);
    printf("CSV Usuarios: %s\n", cfg->usuarios_csv);
    printf("CSV Vehiculos: %s\n", cfg->vehiculos_csv);
    printf("Log: %s\n", cfg->log_path);
    printf("=============================\n");
}