#ifndef BD_H_
#define BD_H_
#include "sqlite3.h"

typedef struct
{
    int id_estacion;
    char nombre;
    char abreviacion[16];
    int plazas;

} Estacion;

typedef struct
{
    int id_vehiculo;
    char estado[23];
    int ubicacion_estacion; // es el ID
    float bateria_restante;

} Vehiculo;// prueba

typedef struct
{
    int id_usuario;
    char nombre;
    char contrasenya;
    int vehiculo_activo;

} Usuario;

typedef struct
{
    int id_averia;
    int id_vehiculo;
    int id_estacion;
    char tipo[32];
    char descripcion;
    char fecha[32];
    char estado;

} Averia;

typedef struct
{
    int id_trayecto;
    int usuario_id;
    int vehiculo_id;
    char inicio[32];
    char fin[32];
    float distancia;
} Trayecto;

typedef struct
{
    int id_reserva;
    int usuario_id;
    int vehiculo_id;
    char hora_inicio[32];
    char hora_final[32];
    char estado[32];

} Reserva;

sqlite3* abrir_baseDatos(const char *ruta); // para abrir o crear el fichero
void cerrar_baseDatos(sqlite3 *db);
int crearTablas(sqlite3 *db);

// carga de csv
int cargar_estaciones(sqlite3 *db, char *scv);
int db_cargar_usuarios(sqlite3 *db, char *scv);
int db_cargar_vehiculos(sqlite3 *db, char *scv);

// para mostrar las estaciones
int db_listarVehiculos(sqlite3 *db);
int db_listar_vehiculos_estacion(sqlite3 *db, int id_estacion);
int db_buscar_vehiculo(sqlite3 *db, int id, Vehiculo *out);
int db_actualizar_estado_vehiculo(sqlite3 *db, int id, char *estado);
int db_actualizar_autonomia(sqlite3 *db, int id, float autonomia);

int db_listar_usuarios(sqlite3 *db);
int db_buscar_usuario_id(sqlite3 *db, int id, Usuario *mostrar);
int db_buscar_usuario_nombre(sqlite3 *db, char *nombre, Usuario *mostrar);
int db_cambiar_contrasenya(sqlite3 *db, int id, char *nueva);
int db_actualizar_vehiculo_activo(sqlite3 *db, int id_usuario, int id_vehiculo);

int db_insertar_averia(sqlite3 *db, Averia *a);
int db_listar_averias_pendientes(sqlite3 *db);
int db_marcar_averia_reparada(sqlite3 *db, int id);
int db_contar_averias_pendientes(sqlite3 *db);

int db_insertar_reserva(sqlite3 *db, Reserva *r);
int db_listar_reservas(sqlite3 *db);

int db_insertar_trayecto(sqlite3 *db, Trayecto *t);
int db_listar_trayectos_usuario(sqlite3 *db, int id_usuario);

void db_estadisticas(sqlite3 *db);

#endif