#ifndef DB_H
#define DB_H

#include "sqlite3.h"
#include <stddef.h>



typedef struct {
    int   id_vehiculo;
    char  estado[32];
    int   ubicacion_estacion;
    float bateria_restante;
} Vehiculo;

typedef struct {
    int  id_usuario;
    char nombre[100];
    char contrasenya[64];
    int  vehiculo_activo;
} Usuario;

typedef struct {
    int  id_averia;
    int  id_vehiculo;
    int  id_estacion;
    char tipo[32];
    char descripcion[256];
    char fecha[32];
    char estado[32];
} Averia;

typedef struct {
    int  id_reserva;
    int  usuario_id;
    int  vehiculo_id;
    char hora_inicio[32];
    char hora_final[32];
    char estado[16];
} Reserva;

typedef struct {
    int    id_trayecto;
    int    usuario_id;
    int    vehiculo_id;
    int    estacion_origen;    
    int    estacion_destino;  
    char   inicio[32];
    char   fin[32];
    double distancia;
} Trayecto;



sqlite3 *abrir_baseDatos(const char *ruta);
void     cerrar_baseDatos(sqlite3 *db);
int      crearTablas(sqlite3 *db);

/* Carga inicial desde CSV */
int cargar_estaciones(sqlite3 *db, const char *csv);
int cargar_usuarios(sqlite3 *db,   const char *csv);
int cargar_vehiculos(sqlite3 *db,  const char *csv);

/* Listados (para el admin local) */
int listar_estaciones(sqlite3 *db);
int listar_vehiculos(sqlite3 *db);
int listar_vehiculosEstacion(sqlite3 *db, int id_estacion);
int listar_usuarios(sqlite3 *db);
int listar_reservas(sqlite3 *db);
int listar_averiasPendientes(sqlite3 *db);
int listar_trayectosUsuario(sqlite3 *db, int id_usuario);

/* Búsquedas */
int buscar_vehiculo(sqlite3 *db, int id, Vehiculo *resultado);
int buscar_usuario_por_id(sqlite3 *db, int id, Usuario *resultado);
int buscar_usuario_por_nombre(sqlite3 *db, const char *nombre, Usuario *resultado);

/* Actualizaciones de vehículo */
int actualizar_estado(sqlite3 *db, int id_vehiculo, const char *estado);
int actualizar_bateria(sqlite3 *db, int id_vehiculo, float bateria);
int actualizar_ubicacion_vehiculo(sqlite3 *db, int id_vehiculo, int id_estacion_destino); /* NEW */
int actualizar_vehiculoActivo(sqlite3 *db, int id_usuario, int id_vehiculo);

/* Usuarios */
int cambiar_contrasenya(sqlite3 *db, int id_usuario, const char *nueva);

/* Averías */
int poner_averia(sqlite3 *db, const Averia *a);
int marcar_reparada(sqlite3 *db, int id_averia);
int contar_pendientes(sqlite3 *db);

/* Reservas */
int insertar_reserva(sqlite3 *db, const Reserva *r);
int reserva_activa_vehiculo(sqlite3 *db, int id_vehiculo);   /* NEW */
int cancelar_reservas_vehiculo(sqlite3 *db, int id_vehiculo); /* NEW */

/* Trayectos */
int insertar_trayecto(sqlite3 *db, const Trayecto *t);

/* Mapas (admin) */
void mapa_grande(sqlite3 *db);
void mapa_pequenyo(sqlite3 *db);
void db_estadisticas(sqlite3 *db);

int capacidad_maxima_estacion(int id_estacion);
int vehiculos_en_estacion(sqlite3 *db, int id_estacion);
int estacion_llena(sqlite3 *db, int id_estacion);

#endif /* DB_H */