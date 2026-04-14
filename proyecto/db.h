#ifndef BD_H_
#define BD_H_

#include "sqlite3.h"

typedef struct {
    int id_estacion;
    char nombre[100];
    char abreviacion[20];
    int plazas;
} Estacion;

typedef struct {
    int id_vehiculo;
    char estado[32];
    int ubicacion_estacion;
    float bateria_restante;
} Vehiculo;

typedef struct {
    int id_usuario;
    char nombre[100];
    char contrasenya[100];
    int vehiculo_activo;
} Usuario;

typedef struct {
    int id_averia;
    int id_vehiculo;
    int id_estacion;
    char tipo[32];
    char descripcion[256];
    char fecha[32];
    char estado[32];
} Averia;

typedef struct {
    int id_trayecto;
    int usuario_id;
    int vehiculo_id;
    char inicio[32];
    char fin[32];
    float distancia;
} Trayecto;

typedef struct {
    int id_reserva;
    int usuario_id;
    int vehiculo_id;
    char hora_inicio[32];
    char hora_final[32];
    char estado[32];
} Reserva;

sqlite3* abrir_baseDatos(const char *ruta);
void cerrar_baseDatos(sqlite3 *db);
int crearTablas(sqlite3 *db);

int cargar_estaciones(sqlite3 *db, const char *csv);
int cargar_usuarios(sqlite3 *db, const char *csv);
int cargar_vehiculos(sqlite3 *db, const char *csv);

int listar_estaciones(sqlite3 *db);
int listar_vehiculosEstacion(sqlite3 *db, int id_estacion);
int listar_usuarios(sqlite3 *db);

int buscar_vehiculo(sqlite3 *db, int id, Vehiculo *resultado);
int buscar_usuario_por_id(sqlite3 *db, int id, Usuario *resultado);
int buscar_usuario_por_nombre(sqlite3 *db, const char *nombre, Usuario *resultado);
int cambiar_contrasenya(sqlite3 *db, int id, const char *nueva);
int actualizar_estado(sqlite3 *db, int id, const char *estado);
int actualizar_bateria(sqlite3 *db, int id, float bateria);
int actualizar_vehiculoActivo(sqlite3 *db, int id_usuario, int id_vehiculo);

int poner_averia(sqlite3 *db, const Averia *a);
int listar_averiasPendientes(sqlite3 *db);
int marcar_reparada(sqlite3 *db, int id_averia);

int listar_reservas(sqlite3 *db);
int insertar_reserva(sqlite3 *db, const Reserva *r);

int insertar_trayecto(sqlite3 *db, const Trayecto *t);
int listar_trayectosUsuario(sqlite3 *db, int id_usuario);
int contar_pendientes(sqlite3 *db); 

void db_estadisticas(sqlite3 *db);

#endif
