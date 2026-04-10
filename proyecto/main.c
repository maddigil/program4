#include <stdio.h>
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
    
    printf("\n Configuracion cargada: \n");
    config_mostrar(&cfg);

    sqlite3 *db = db_abrir(cfg.db_path);
    if(!db) return 1;

    if (!db_crear_tablas(db)){
        db_cerrar(db);
        return 1;
    }

    printf("\n Cargando datos iniciales...\n");
    db_cargar_estaciones(db, cfg.estaciones_csv);
    db_cargar_usuarios(db, cfg.usuarios_csv);
    db_cargar_vehiculos(db, cfg.vehiculos_csv);

    if(!admin_login(&cfg)){
        db_cerrar(db);
        return 1;
    }

    admin_menu(db, &cfg);
    db_cerrar(db);

    return 0;
}