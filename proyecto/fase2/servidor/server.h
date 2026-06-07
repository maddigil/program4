/*
 * server.h
 * Declaraciones del servidor de sockets de Euskokar.
 * El servidor corre en un hilo (pthread) separado al menu de administrador,
 * de modo que ambos pueden funcionar a la vez.
 */

#ifndef SERVER_H
#define SERVER_H

#include "sqlite3.h"
#include "config.h"
#include <pthread.h>

/* Datos que necesita el hilo servidor */
typedef struct {
    sqlite3          *db;          /* Base de datos compartida */
    const Config     *cfg;         /* Configuracion del sistema */
    int               puerto;      /* Puerto en el que escucha */
    pthread_mutex_t  *mutex_db;    /* Mutex para proteger acceso a la BD */
} ServidorArgs;

/*
 * servidor_iniciar:
 *   Lanza el servidor de sockets en un hilo aparte.
 *   Devuelve 1 si se inicio bien, 0 si hubo error.
 *   El hilo queda guardado en *hilo para poder hacer join al salir.
 */
int servidor_iniciar(ServidorArgs *args, pthread_t *hilo);

#endif /* SERVER_H */
