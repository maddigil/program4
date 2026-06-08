#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <winsock2.h>
  #include <windows.h>
#else
  #include <unistd.h>
  #include <pthread.h>
#endif


#include "config.h"
#include "db.h"
#include "admin.h"
#include "logic.h"
#include "server.h"

int main(void) {
    Config cfg;
    memset(&cfg, 0, sizeof(cfg));

    if (!config_cargar("datos/config.cfg", &cfg)) {
        fprintf(stderr, "Error al cargar datos/config.cfg\n");
        return 1;
    }

    printf("\n");
    config_mostrar(&cfg);

    sqlite3 *db = abrir_baseDatos(cfg.db_path);
    if (!db) {
        fprintf(stderr, "Error al abrir la base de datos: %s\n", cfg.db_path);
        config_liberar(&cfg);
        return 1;
    }

    if (!crearTablas(db)) {
        fprintf(stderr, "Error creando tablas\n");
        cerrar_baseDatos(db);
        config_liberar(&cfg);
        return 1;
    }

    printf("\nCargando datos iniciales...\n");
    cargar_estaciones(db, cfg.estaciones_csv);
    cargar_usuarios(db,   cfg.usuarios_csv);
    cargar_vehiculos(db,  cfg.vehiculos_csv);

    pthread_mutex_t mutex_db;
    pthread_mutex_init(&mutex_db, NULL);

    int puerto = cfg.servidor_puerto > 0 ? cfg.servidor_puerto : PUERTO_DEFAULT;

    ServidorArgs args_srv;
    args_srv.db       = db;
    args_srv.cfg      = &cfg;
    args_srv.puerto   = puerto;
    args_srv.mutex_db = &mutex_db;

    pthread_t hilo_srv;
    if (!servidor_iniciar(&args_srv, &hilo_srv)) {
        fprintf(stderr, "Error al lanzar el servidor de sockets\n");
        cerrar_baseDatos(db);
        config_liberar(&cfg);
        return 1;
    }

#ifdef _WIN32
    Sleep(100);
#endif

    printf("[SERVIDOR] Escuchando en puerto %d...\n\n", puerto);
    printf("Iniciando sesion de administrador local...\n");
    printf("(Usuario admin: '%s' segun datos/config.cfg)\n\n", cfg.admin_usuario ? cfg.admin_usuario : "admin");

    if (!admin_login(&cfg)) {
        printf("Acceso denegado. El servidor sigue activo en segundo plano.\n");
#ifdef _WIN32
        printf("Pulsa Ctrl+C para detener el servidor.\n");
        WaitForSingleObject(hilo_srv, INFINITE);
#else
        pause();
#endif
        pthread_mutex_destroy(&mutex_db);
        cerrar_baseDatos(db);
        config_liberar(&cfg);
        return 1;
    }

    admin_menu(db, &cfg);

    /* Al salir del menu admin el servidor de sockets sigue activo */
    printf("\nSesion admin cerrada. El servidor sigue activo.\n");
    printf("Pulsa Ctrl+C para apagar el servidor.\n");
#ifdef _WIN32
    WaitForSingleObject(hilo_srv, INFINITE);
#else
    pause();
#endif
    pthread_mutex_destroy(&mutex_db);
    cerrar_baseDatos(db);
    config_liberar(&cfg);
    return 0;
}
