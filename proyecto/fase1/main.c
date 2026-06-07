#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "db.h"
#include "admin.h"
#include "logic.h"

int main(void){
    Config cfg;
    
    if(!config_cargar("config.cfg", &cfg)){
        fprintf(stderr, "Error al cargar config.cfg\n");
        return 1;
    }
    
    printf("\n");
    config_mostrar(&cfg);

    sqlite3 *db = abrir_baseDatos(cfg.db_path);
    if(!db){
        fprintf(stderr, "Error al abrir base de datos: %s\n", cfg.db_path);
        return 1;
    }

    if(!crearTablas(db)){
        fprintf(stderr, "Error creando tablas\n");
        cerrar_baseDatos(db);
        return 1;
    }

    printf("\nCargando datos iniciales...\n");
    cargar_estaciones(db, cfg.estaciones_csv);
    cargar_usuarios(db, cfg.usuarios_csv);
    cargar_vehiculos(db, cfg.vehiculos_csv);

    printf("\nIniciando sesion de administrador...\n");
    if(!admin_login(&cfg)){
        printf("Acceso denegado\n");
        cerrar_baseDatos(db);
        return 1;
    }

    admin_menu(db, &cfg);
    cerrar_baseDatos(db);

    /*liberar memoria de configuración*/
    config_liberar(&cfg);

    printf("\nPrograma finalizado correctamente.\n");
    return 0;
}