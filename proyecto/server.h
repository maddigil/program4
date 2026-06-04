#ifndef SERVER_H
#define SERVER_H

#include "sqlite3.h"
#include "config.h"
#include <pthread.h>

typedef struct {
    sqlite3          *db;          /* Base de datos compartida */
    const Config     *cfg;         /* Configuracion del sistema */
    int               puerto;      /* Puerto en el que escucha */
    pthread_mutex_t  *mutex_db;    /* Mutex para proteger acceso a la BD */
} ServidorArgs;


int servidor_iniciar(ServidorArgs *args, pthread_t *hilo);

#endif 