#ifndef CONFIG_H
#define CONFIG_H

/*tamaño max para ruta de fichero*/
#define MAX_RUTA 256

/*tmaño max para valores en consola*/
#define MAX_VALOR 128

typedef struct {
    char admin_usuario[MAX_VALOR];  /*user del admin */
    char admin_clave[MAX_VALOR];    /*contraseña del admin */
    char db_path[MAX_RUTA];         /*ruta del fichero de base de datos */
    char estaciones_csv[MAX_RUTA];  /*ruta del csv de estaciones */
    char usuarios_csv[MAX_RUTA];    /*ruta del csv de usuarios */
    char vehiculos_csv[MAX_RUTA];   /*ruta del csv de vehículos */
    char log_path[MAX_RUTA];        /*ruta del log */
} Config;

/*para leer el fichero de config.cfg y rellenar aquí, devuelve 1 si se ejecuta bien, sino 0*/
/*se utiliza el const para que no haya errores de compilación, no funcinaría sin él*/
int config_cargar(const char *ruta, Config *cfg);

/*imprime en consola lo que has cargado antes*/
/*se utiliza el const para que aseguremos que no haya errores luego, funcionaría sin él*/
void config_mostrar(const Config *cfg);

#endif