#include "logic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_STR 128

void fecha_ahora(char *buf, int size){
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

void log_escribir(const Config *cfg, const char *mensaje){
    FILE *f = fopen(cfg->log_path, "a");
    if(!f) return;
    char fecha[32];
    fecha_ahora(fecha, sizeof(fecha));
    fprintf(f, "[%s] %s\n", fecha, mensaje);
    fclose(f);
}

static void limpiar_buffer(void){
    int c;
    while((c = getchar()) != '\n' && c != EOF);
}

int leer_entero(const char *prompt, int min, int max){
    int val;
    while(1){
        printf("%s", prompt);
        if(scanf("%d", &val) == 1 && val >= min && val <= max){
            limpiar_buffer();
            return val;
        }
        limpiar_buffer();
        printf("Valor invalido. Introduce un numero entre %d y %d.\n", min, max);
    }
}

void leer_cadena(const char *prompt, char *buf, int size){
    printf("%s", prompt);
    if(fgets(buf, size, stdin) == NULL){
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\n")] = '\0';
}

int logic_registrar_averia(sqlite3 *db, const Config *cfg){
    printf("\n  +-----------------------------+\n");
    printf("  |   REGISTRAR NUEVA AVERIA    |\n");
    printf("  +-----------------------------+\n");
    printf("  1. Averia de vehiculo\n");
    printf("  2. Averia de estacion\n");

    int tipo_obj = leer_entero(" Selecciona (1-2): ", 1, 2);

    Averia a;
    memset(&a, 0, sizeof(a));
    strcpy(a.estado, "pendiente");
    fecha_ahora(a.fecha, sizeof(a.fecha));

    if(tipo_obj == 1){
        a.id_vehiculo = leer_entero(" ID del vehiculo: ", 1, 9999);
        a.id_estacion = 0;

        Vehiculo v;
        if(!buscar_vehiculo(db, a.id_vehiculo, &v)){
            printf("ERROR: Vehiculo %d no encontrado\n", a.id_vehiculo);
            return 0;
        }

        actualizar_estado(db, a.id_vehiculo, "averiado");
        printf("Vehiculo %d marcado como averiado.\n", a.id_vehiculo);
    }else{
        a.id_estacion = leer_entero(" ID de la estacion: \n", 1, 9999);
        a.id_vehiculo = 0;
    }

    printf("Tipo de averia:\n");
    printf("  1. mecanica\n  2. bateria\n  3. carga\n  4. otro\n");
    int t = leer_entero(" Selecciona (1-4): ", 1, 4);
    const char *tipos[] = {"mecanica", "bateria", "carga", "otro"};
    strncpy(a.tipo, tipos[t - 1], sizeof(a.tipo) - 1);
    a.tipo[sizeof(a.tipo) - 1] = '\0';

    leer_cadena(" Descripcion: ", a.descripcion, sizeof(a.descripcion));

    if(poner_averia(db, &a)){
        printf("Averia registrada correctamente.\n");
        char msg[512];
        snprintf(msg, sizeof(msg), "AVERIA registrada - vehiculo:%d estacion:%d tipo:%s",
                 a.id_vehiculo, a.id_estacion, a.tipo);
        log_escribir(cfg, msg);
        return 1;
    }

    printf("ERROR al registrar la averia.\n");
    return 0;
}

int logic_cambiar_estado_vehiculo(sqlite3 *db, const Config *cfg){
    printf("\n  +----------------------------------+\n");
    printf("  |   CAMBIAR ESTADO DE VEHICULO     |\n");
    printf("  +----------------------------------+\n");

    int id = leer_entero(" ID del vehiculo: ", 1, 9999);
    Vehiculo v;

    if(!buscar_vehiculo(db, id, &v)){
        printf("ERROR: Vehiculo %d no encontrado\n", id);
        return 0;
    }

    printf("Estado actual: %s | Bateria: %.1f%%\n", v.estado, v.bateria_restante);

    printf("Nuevo estado:\n");
    printf(" 1. disponible\n 2. en uso\n 3. averiado\n");
    int opcion = leer_entero(" Selecciona (1-3): ", 1, 3);
    const char *estados[] = {"disponible", "en uso", "averiado"};

    if(actualizar_estado(db, id, estados[opcion - 1])){
        printf("Estado actualizado a '%s'.\n", estados[opcion - 1]);
        char msg[256];
        snprintf(msg, sizeof(msg), "Vehiculo %d estado cambiado a %s", id, estados[opcion - 1]);
        log_escribir(cfg, msg);
        return 1;
    }

    printf("ERROR al actualizar el estado del vehiculo.\n");
    return 0;
}

int logic_cambiar_contrasena_usuario(sqlite3 *db){
    printf("\n  +----------------------------------+\n");
    printf("  |   CAMBIAR CONTRASENA USUARIO     |\n");
    printf("  +----------------------------------+\n");

    int id = leer_entero(" ID del usuario: ", 1, 9999);

    Usuario u;
    if(!buscar_usuario_por_id(db, id, &u)){
        printf("ERROR: Usuario %d no encontrado\n", id);
        return 0;
    }

    printf("Usuario: %s\n", u.nombre);

    char nueva[MAX_STR];
    leer_cadena(" Nueva contrasena: ", nueva, MAX_STR);

    if(strlen(nueva) < 1){
        printf("Contrasena vacia, operacion cancelada.\n");
        return 0;
    }

    if(cambiar_contrasenya(db, id, nueva)){
        printf("Contrasena actualizada correctamente.\n");
        return 1;
    }

    printf("ERROR al actualizar contrasena.\n");
    return 0;
}

void logic_mostrar_log(const Config *cfg){
    FILE *f = fopen(cfg->log_path, "r");
    if(!f){
        printf("(El fichero del log esta vacio o no existe aun)\n");
        return;
    }

    printf("\n === CONTENIDO DEL LOG (%s) ===\n\n", cfg->log_path);
    char linea[512];
    int n = 0;

    while(fgets(linea, sizeof(linea), f)){
        printf("%s", linea);
        n++;
    }

    fclose(f);

    if(!n){
        printf("(log vacio)\n");
    }

    printf("\nTotal: %d entradas.\n", n);
}