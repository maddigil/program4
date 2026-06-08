#ifndef SERVER_H
#define SERVER_H

#include "sqlite3.h"
#include "config.h"

#ifdef _WIN32
  #include <winsock2.h>
  #include <windows.h>
  typedef HANDLE     pthread_t;
  typedef HANDLE     pthread_mutex_t;
  #define pthread_mutex_lock(m)    WaitForSingleObject(*(m), INFINITE)
  #define pthread_mutex_unlock(m)  ReleaseMutex(*(m))
  #define pthread_mutex_init(m,a)  (*(m) = CreateMutex(NULL, FALSE, NULL))
  #define pthread_mutex_destroy(m) CloseHandle(*(m))
  #define close(s)                 closesocket(s)
#else
  #include <pthread.h>
#endif

typedef struct {
    sqlite3         *db;
    const Config    *cfg;
    int              puerto;
    pthread_mutex_t *mutex_db;
} ServidorArgs;

int servidor_iniciar(ServidorArgs *args, pthread_t *hilo);

#endif
