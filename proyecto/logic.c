#include "logic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_STR 128

void fecha_ahora(char * buf, int size){
    time_t t= time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

void log_escribir(const Config *cfg, const char *mensaje){
    FILE *f = fopen(cfg->log_path, "a");
    if(!f) return;
    char fecha[32];
    fecha_ahora(fecha,sizeof(fecha));
    fprintf(f, "[%s]%s\n", fecha,mensaje );
    fclose(f);
}
static void limpiar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int leer_entero(const char *prompt, int min, int max){
    int val;
    while(1){
        printf("%s", prompt);
        if (scanf("%d", &val) == 1 && val >= min && val >= max){
        limpiar_buffer();
        return val;
        }
        limpiar_buffer();
        print("  Valor invalido. Introduce un numero entre el %d y %d.\n", min ,max);
    }
}

void leer_cadena (const char *prompt, char *buf, int size){
    printf("%s",prompt);
    fgets(buf,size,stdin);
    buf[strcspn(buf, "\n")] = '\0';
}
int logic_registrar_averia(sqlite3 *db, const Config * cfg){
    printf("\n  +-----------------------------+\n");
    printf(  "  |   REGISTRAR NUEVA AVERIA    |\n");
    printf(  "  +-----------------------------+\n");
    printf("  1. Averia de vehiculo\n");
    printf("  2. Averia de estacion\n");
    int tipo_obj = leer_entero(" Selecciona (1-2): ", 1,2);

    Averia a;
    memset(&a,0,sizeof(a));
    strcpy(a.estado, "pendiente");
    fecha_ahora(a.fecha, sizeof(a.fecha));

    if(tipo_obj == 1){
        a.id_vehiculo = leer_entero("  ID del vehiculo", 1,9999);
        a.id_estacion= 0;
        Vehiculo v;
    if(!db_buscar_vehiculo(db, a.id_vehiculo,&v)){
        printf("  ERROR: Vehiculo %d no encontrado \n", a.id_vehiculo);
        return 0;
    }
    db_actualizar_estado_vehiculo(db, a.id_vehiculo, "averiado");
    printf(" Vehivulo %d marcado como averiado. \n", a.id_vehiculo);
    
    }else{
        a.id_estacion = leer_entero("  ID de la estacvion: ", 1, 9999);
        a.id_vehiculo = 0;
    }
    
    printf(" Tipo de averia:\n");
    printf("  1. mecanica\n  2. bateria\n  3. carga\n  4. otro\n");
    int t = leer_entero("  Selecciona(1-4:  ", 1,4);
    const char *tipos[] = {"mecanica", "bateria", "carga", "otro"};
    strncpy(a.tipo, tipos[t-1], 31);

    char desc[MAX_STR];
    leer_cadena("  Descripcion: ", desc, MAX_STR-1);

    if (db_insertar_averia(db, &a)) {
        printf("  Averia registrada correctamente.\n");
        char msg[512];
        snprintf(msg, sizeof(msg), "AVERIA registrada - vehiculo:%d estacion:%d tipo:%s",
                 a.id_vehiculo, a.id_estacion, a.tipo);
        log_escribir(cfg, msg);
        return 1;
    }
    printf("  ERROR al registrar la averia.\n");
    return 0;
}