#include "admin.h"
#include "db.h"
#include "logic.h"
#include <stdio.h>
#include <string.h>

int admin_login(const Config *cfg){
    char usuario[MAX_VALOR], clave[MAX_VALOR];

     printf("\n  +======================================+\n");
    printf(  "  |    SISTEMA DE GESTION EUSKOKAR       |\n");
    printf(  "  |         ADMINISTRADOR LOCAL          |\n");
    printf(  "  +======================================+\n\n");

    int intentos=3;

    while(intentos > 0){
        intentos--;
        leer_cadena(" Usuario: ", usuario, MAX_VALOR-1);
        leer_cadena(" Clave  : ", clave, MAX_VALOR-1);
        if(strcmp(usuario, cfg->admin_usuario)==0 && strcmp(clave, cfg->admin_clave)==0){
            printf("Bienvenico, %s \n", usuario);
            return 1;
        }
    
    printf("Datos incorrectos: intentos restantes %i\n", intentos);
    }
    printf("Demasioados intentos fallidos. Saliendo.\n");
    return 0;
}

static void menu_vehiculos(sqlite3 *db, const Config *cfg){
    int op;
    while(op!=0){
        printf("\n  +-----------------------------+\n");
        printf(  "  |       GESTION VEHICULOS     |\n");
        printf(  "  +-----------------------------+\n");
        printf("  1. Listar todos los vehiculos\n");
        printf("  2. Vehiculos de una estacion\n");
        printf("  3. Cambiar estado de vehiculo\n");
        printf("  4. Actualizar autonomia (bateria)\n");
        printf("  0. Volver\n");

        op= leer_entero(" Opción: ", 0, 4);

        if(op==1){
            db_listar_vehiculos(db);
        }else if(op==2){
            int id=leer_entero(" ID de estacion: ", 1, 9999);
            db_listar_vehiculos_estacion(db,id);
        } else if(op==3){
            logic_cambiar_estado_vehiculo(db, cfg);
        }else if(op==4){
            int id =leer_entero(" ID del vehiculo: ", 1, 9999);
            Vehiculo v;
            if(db_buscar_vehiculo(db, id, &v)){
                printf("Autonomía actual: %.1f%%\n", v.bateria_restante);
                int nueva=leer_entero(" Nueva autonomia(0-100): ", 0, 100);
                db_actualizar_autonomia(db, id, (float)nueva);
                printf("Autonomia actualizada \n");
                char msg[128];
                snprintf(msg, sizeof(msg), "VEHICULO %i autonomia actualizada a %d\n", id, nueva);
                log_escribir(cfg, msg);
            }else{
                prinft("vehiculo no encontrado\n");
            }
        }
    }
}

static void menu_usuarios(sqlite3 *db){
    int op;
    while(op!=0){
        printf("\n  +-----------------------------+\n");
        printf(  "  |       GESTION USUARIOS       |\n");
        printf(  "  +-----------------------------+\n");
        printf("  1. Listar todos los usuarios\n");
        printf("  2. Buscar usuario por ID\n");
        printf("  3. Cambiar contrasena de usuario\n");
        printf("  0. Volver\n");
        op=leer_entero(" Opcion: ", 0, 3);
        if(op==1){
            db_listar_usuarios(db);
        }else if(op==2){
            int id=leer_entero(" ID del usuario: ", 1, 99999);
            Usuario u;
            if(db_buscar_usuario_id(db, id, &u)){
                printf(" ID: %d\n", u.id_usuario);
                printf(" Nombre: %s\n", u.nombre);
                if (u.vehiculo_activo){
                   printf("  Veh.activo: Si\n");
                }else{
                printf("  Veh.activo: No\n");}
            }else{
                printf("Usuario no encontrado \n");
            }
        }else if(op==3){
            logic_cambiar_contrasena_usuario(db);
        }
    }
}

static void menu_averias(sqlite3 *db, const Config *cfg){
    int op;
    while(op!=0){
        int pendientes = db_contar_averias_pendientes(db);
        printf("\n  +-----------------------------+\n");
        printf(  "  |       GESTION AVERIAS       |\n");
        printf(  "  +-----------------------------+\n");
        printf("  Averias pendientes: %d\n\n", pendientes);
        printf("  1. Ver averias pendientes\n");
        printf("  2. Registrar nueva averia\n");
        printf("  3. Marcar averia como reparada\n");
        printf("  0. Volver\n");
        op= leer_entero(" Opcion: ", 0, 3);

        if(op==1){
            db_listar_averias_pendientes(db);
        }else if(op==2){
            logic_registrar_averia(db,cfg);
        }else if(op==3){
            db_listar_averias_pendientes(db);
            int id=leer_entero(" ID de la averia a marcar reparada: ", 1, 99999);
            if(db_marcar_averia_reparada(db, id)){
                prinf("Averia %i marcada como reparada \n", id);
                char msg[128];
                snprintf(msg, sizeof(msg), "AVERIA %i marcada como reparada\n", id);
                log_escribir(cfg, msg);
            } else{
                printf("No se encontro la averia o ya estaba reparada\n");
            }
        }
        
    }
}

static void menu_estaciones(sqlite3 *db){
    int op;
    while(op!=0){
        printf("\n  +-----------------------------+\n");
        printf(  "  |      GESTION ESTACIONES     |\n");
        printf(  "  +-----------------------------+\n");
        printf("  1. Listar todas las estaciones\n");
        printf("  2. Ver vehiculos de una estacion\n");
        printf("  0. Volver\n");
        op = leer_entero("  Opcion: ", 0, 2);

        if(op==1){
            db_listar_estaciones(db);
        }else if(op==2){
            db_listar_estaciones(db);
            int id =leer_entero(" ID de estacion: ", 1, 9999);
            db_listar_vehiculos_estacion(db, id);
        }
    }
}


void admin_menu(sqlite3 *db, const Config *cfg){
    log_escribir(cfg, "SESION iniciada por administrador");
    int op;
    while(op!=0){
        printf("\n  +======================================+\n");
        printf(  "  |        MENU PRINCIPAL ADMIN          |\n");
        printf(  "  +======================================+\n");
        printf("  1. Gestion de estaciones\n");
        printf("  2. Gestion de vehiculos\n");
        printf("  3. Gestion de usuarios\n");
        printf("  4. Gestion de averias\n");
        printf("  5. Estadisticas del sistema\n");
        printf("  6. Ver log del sistema\n");
        printf("  7. Ver reservas\n");
        printf("  0. Salir\n");
        op = leer_entero("  Opcion: ", 0, 7);

        if(op==1){
            menu_estaciones(db);
            break;
        }else if(op==2){
            menu_vehiculos(db, cfg);
            break;
        } else if(op==3){
            menu_usuarios(db);
            break;
        }else if(op==4){
            menu_averias(db, cfg);
            break;
        }else if(op==5){
            db_estadisticas(db);
            break;
        }else if(op==6){
            logic_mostrar_log(cfg);
            break;
        }else if (op==7){
            db_listar_reservas(db);
            break;
        }
    }
    log_escribir(cfg, "SESION cerradoa po administrador");
    printf("\n Sesion cerrada. ¡Hasta pronto!");

}