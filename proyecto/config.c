#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 

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

       char **destino = NULL;

        if(strcmp(nombre, "admin_usuario") == 0){
            destino = &cfg->admin_usuario;
        }
        else if(strcmp(nombre, "admin_clave") == 0){
            destino = &cfg->admin_clave;
        }
        else if(strcmp(nombre, "db_path") == 0){
            destino = &cfg->db_path;
        }
        else if(strcmp(nombre, "estaciones_csv") == 0){
            destino = &cfg->estaciones_csv;
        }
        else if(strcmp(nombre, "usuarios_csv") == 0){
            destino = &cfg->usuarios_csv;
        }
        else if(strcmp(nombre, "vehiculos_csv") == 0){
            destino = &cfg->vehiculos_csv;
        }
        else if(strcmp(nombre, "log_path") == 0){
            destino = &cfg->log_path;
        }

        if(destino){
            free(*destino); // liberar si ya tenía memoria
            *destino = malloc(strlen(valor) + 1);
            if(*destino){
                strcpy(*destino, valor);
            }
        }

    fclose(f);
    return 1;
}

void config_mostrar(const Config *cfg){
    printf("=== CONFIGURACION CARGADA ===\n");
    //printf("Admin: %s / %s\n", cfg->admin_usuario, cfg->admin_clave);
    printf("DB: %s\n", cfg->db_path);
    printf("CSV Estaciones: %s\n", cfg->estaciones_csv);
    printf("CSV Usuarios: %s\n", cfg->usuarios_csv);
    printf("CSV Vehiculos: %s\n", cfg->vehiculos_csv);
    printf("Log: %s\n", cfg->log_path);
    printf("=============================\n");
}

void config_liberar(Config *cfg){
    free(cfg->admin_usuario);
    free(cfg->admin_clave);
    free(cfg->db_path);
    free(cfg->estaciones_csv);
    free(cfg->usuarios_csv);
    free(cfg->vehiculos_csv);
    free(cfg->log_path);

    // Opcional: poner a NULL
    cfg->admin_usuario = NULL;
    cfg->admin_clave = NULL;
    cfg->db_path = NULL;
    cfg->estaciones_csv = NULL;
    cfg->usuarios_csv = NULL;
    cfg->vehiculos_csv = NULL;
    cfg->log_path = NULL;
}
