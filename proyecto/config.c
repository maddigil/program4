#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*es static porque solo la usamos en este archivo*/
static void trim(char *s){
    int len=(int)strlen(s);
    while(len>0 && (s[len-1]=='\n'|| s[len-1]=='\r'||s[len-1]==' ')){
        s[--len]='\0';
    }
}

/*abrir y leer el fichero*/
int config_cargar(const char *ruta, Config *cfg){
    FILE *f =fopen(ruta, "r");
    if(!f){
        fprintf(stderr, "no se ha podido abrir %s ", ruta);
    return 0;}

    /*por si alguna clave no está, valores por defecto*/
    strcpy(cfg->admin_usuario, "admin");
    strcpy(cfg->admin_clave, "admin123");
    strcpy(cfg->db_path, "gipuzkoa.db");
    strcpy(cfg->estaciones_csv, "estaciones.csv");
    strcpy(cfg->usuarios_csv, "usuarios.csv");
    strcpy(cfg->vehiculos_csv, "vehiculos.csv");

    /*guardar la linea leida*/
    char linea[512];
    /*leemos el fichero*/
    while(fgets(linea, sizeof(linea), f)){
        /*limpiar el salto de linea*/
        trim(linea);
        /*ignoramos lineas vacias y comentarios */
        if(linea[0]=='#'|| linea[0]='\0'){
            continue;
        }
        /*usamos el = para separar; lee 127 char hasta = y luego otros 127*/
        char clave[MAX_VALOR], valor[MAX_VALOR];
    
        if(sscanf(linea, "%127[^=]=%127s", clave, valor)==2){
            /*limpiar posibles espacios en clave y valor*/
            trim(clave);
            trim(valor);

            if(!strcmp(clave, "admin_usuario")){
                strncpy(cfg->admin_usuario, valor, MAX_VALOR-1);
            }else if(!strcmp(clave, "admin_clave")){
                strncpy(cfg->admin_clave, valor, MAX_VALOR-1);
            }else if (!strcmp(clave,"db_path")){
                strncpy(cfg->db_path, valor, MAX_RUTA-1);
            }else if (!strcmp(clave,"estaciones_csv")){
                strncpy(cfg->estaciones_csv, valor, MAX_RUTA-1);
            }else if (!strcmp(clave,"usuarios_csv")){
                strncpy(cfg->usuarios_csv, valor, MAX_RUTA-1);
            }else if (!strcmp(clave,"vehiculos_csv")){
                strncpy(cfg->vehiculos_csv, valor, MAX_RUTA-1);
            }
        }
    }
    fclose(f);

    return 1;

}

/*imprime valores en config*/
void config_mostrar(const Config *cfg) {
    printf("Admin     : %s\n", cfg->admin_usuario);   /* user admin */
    printf("Base datos: %s\n", cfg->db_path);         /*fichero .db */
    printf("Estaciones: %s\n", cfg->estaciones_csv);  /*csv estaciones */
    printf("Usuarios  : %s\n", cfg->usuarios_csv);    /*csv usuarios */
    printf("Vehiculos : %s\n", cfg->vehiculos_csv);   /*csv vehiculos */
    printf("Log: %s\n", cfg->log_path);               /* fichero log */
}

