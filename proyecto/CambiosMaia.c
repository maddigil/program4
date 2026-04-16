// cambios en db.h

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

int listar_vehiculos(sqlite3 *db);
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
void esquema_grande(sqlite3 *db);
void esquema_pequenyo(sqlite3 *db);

void db_estadisticas(sqlite3 *db);
void mapa_grande(sqlite3 *db);
void mapa_pequenyo(sqlite3 *db);


#endif


//cambio en db.c

#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

sqlite3* abrir_baseDatos(const char *ruta){
    sqlite3 *db;
    if (sqlite3_open(ruta, &db) != SQLITE_OK){
        printf("Ha habido un error al abrirlo '%s': %s\n", ruta, sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }
    return db;
}

void cerrar_baseDatos(sqlite3 *db){
    sqlite3_close(db);
}

int crearTablas(sqlite3 *db){
    const char *sql =
        "CREATE TABLE IF NOT EXISTS Estacion ("
        "  id_estacion INTEGER PRIMARY KEY,"
        "  nombre TEXT NOT NULL,"
        "  abreviacion TEXT,"
        "  plazas INTEGER DEFAULT 0"
        ");"
        "CREATE TABLE IF NOT EXISTS Vehiculo ("
        "  id_vehiculo INTEGER PRIMARY KEY,"
        "  estado TEXT DEFAULT 'disponible',"
        "  ubicacion_estacion INTEGER REFERENCES Estacion(id_estacion),"
        "  bateria REAL DEFAULT 100.0"
        ");"
        "CREATE TABLE IF NOT EXISTS Usuario ("
        "  id_usuario INTEGER PRIMARY KEY,"
        "  nombre TEXT NOT NULL,"
        "  contrasenya TEXT NOT NULL DEFAULT '1234',"
        "  vehiculo_activo INTEGER DEFAULT 0"
        ");"
        "CREATE TABLE IF NOT EXISTS Averia ("
        "  id_averia INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  id_vehiculo INTEGER DEFAULT 0,"
        "  id_estacion INTEGER DEFAULT 1,"
        "  tipo TEXT,"
        "  descripcion TEXT,"
        "  fecha_reporte TEXT,"
        "  estado TEXT DEFAULT 'pendiente'"
        ");"
        "CREATE TABLE IF NOT EXISTS Reserva ("
        "  id_reserva INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  usuario_id INTEGER REFERENCES Usuario(id_usuario),"
        "  vehiculo_id INTEGER REFERENCES Vehiculo(id_vehiculo),"
        "  hora_inicio TEXT,"
        "  hora_fin TEXT,"
        "  estado TEXT DEFAULT 'activa'"
        ");"
        "CREATE TABLE IF NOT EXISTS Trayecto ("
        "  id_trayecto INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  usuario_id INTEGER REFERENCES Usuario(id_usuario),"
        "  vehiculo_id INTEGER REFERENCES Vehiculo(id_vehiculo),"
        "  inicio TEXT,"
        "  fin TEXT,"
        "  distancia REAL DEFAULT 0.0"
        ");";

    if(sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK){
        printf("Ha habido un error creando tablas\n");
        return 0;
    }
    return 1;
}

static void limpiarCsv(char *string){
    for(int i = 0; string[i] != '\0'; i++){
        if(string[i] == '\r' || string[i] == '\n'){
            string[i] = '\0';
        }
    }
}

int cargar_estaciones(sqlite3 *db, const char *csv){
    FILE *fd = fopen(csv, "r");
    if(fd == NULL){
        perror("Error");
        exit(1);
    }

    char linea[256];
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "INSERT INTO Estacion VALUES(?,?,?,?);", -1, &stmt, NULL);
    int contador = 0;

    while(fgets(linea, sizeof(linea), fd)){
        limpiarCsv(linea);

        int id;
        int plazas;
        char nombre[100];
        char abrev[20];

        if(sscanf(linea, "%d,%99[^,],%19[^,],%d", &id, nombre, abrev, &plazas) == 4){
            sqlite3_bind_int(stmt, 1, id);
            sqlite3_bind_text(stmt, 2, nombre, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, abrev, -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 4, plazas);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
            contador++;
        }
    }

    sqlite3_finalize(stmt);
    fclose(fd);
    printf("Se han cargado %d estaciones.\n", contador);
    return contador;
}

int cargar_usuarios(sqlite3 *db, const char *csv){
    FILE *fd = fopen(csv, "r");
    if(fd == NULL){
        perror("Error");
        exit(1);
    }

    char linea[256];
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO Usuario VALUES(?,?, '1234',0);", -1, &stmt, NULL);
    int contador = 0;

    while(fgets(linea, sizeof(linea), fd)){
        limpiarCsv(linea);

        int id;
        char nombre[100];

        if(sscanf(linea, "%d,%99[^\n]", &id, nombre) == 2){
            sqlite3_bind_int(stmt, 1, id);
            sqlite3_bind_text(stmt, 2, nombre, -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
            contador++;
        }
    }

    sqlite3_finalize(stmt);
    fclose(fd);
    printf("Se han cargado %d usuarios.\n", contador);
    return contador;
}

int cargar_vehiculos(sqlite3 *db, const char *csv){
    FILE *fd = fopen(csv, "r");
    if(fd == NULL){
        perror("Error");
        exit(1);
    }

    char linea[256];
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO Vehiculo VALUES(?,?,?,?);", -1, &stmt, NULL);
    int contador = 0;

    while(fgets(linea, sizeof(linea), fd)){
        limpiarCsv(linea);

        int id;
        int id_estacion;
        char estado[50];
        float bateria;

        if(sscanf(linea, "%d,%49[^,],%d,%f", &id, estado, &id_estacion, &bateria) == 4){
            sqlite3_bind_int(stmt, 1, id);
            sqlite3_bind_text(stmt, 2, estado, -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 3, id_estacion);
            sqlite3_bind_double(stmt, 4, bateria);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
            contador++;
        }
    }

    sqlite3_finalize(stmt);
    fclose(fd);
    printf("Se han cargado %d vehiculos.\n", contador);
    return contador;
}

int listar_estaciones(sqlite3 *db){
    sqlite3_stmt *stmt;
    int contador = 0;
    sqlite3_prepare_v2(db, "SELECT id_estacion, abreviacion, nombre, plazas FROM Estacion;", -1, &stmt, NULL);

    while(sqlite3_step(stmt) == SQLITE_ROW){
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *abrev = sqlite3_column_text(stmt, 1);
        const unsigned char *nom = sqlite3_column_text(stmt, 2);
        int plazas = sqlite3_column_int(stmt, 3);
        printf("%d %s %s %d\n", id, abrev ? (const char *)abrev : "", nom ? (const char *)nom : "", plazas);
        contador++;
    }

    sqlite3_finalize(stmt);
    printf("\n%d estaciones encontradas\n", contador);
    return contador;
}

int listar_vehiculos(sqlite3 *db){
    sqlite3_stmt *stmt;
    int contador = 0;
    sqlite3_prepare_v2(db,
        "SELECT v.id_vehiculo, v.estado, v.bateria, e.abreviacion "
        "FROM Vehiculo v LEFT JOIN Estacion e ON v.ubicacion_estacion = e.id_estacion;",
        -1, &stmt, NULL);

    while(sqlite3_step(stmt) == SQLITE_ROW){
        int id = sqlite3_column_int(stmt, 0);
        const char *estado = (const char *)sqlite3_column_text(stmt, 1);
        double bat = sqlite3_column_double(stmt, 2);
        const char *estacion = (const char *)sqlite3_column_text(stmt, 3);
        printf("%d %s %.2f %s\n", id, estado ? (const char*) estado : "", bat, estacion ? (const char*) estacion : "-");
        contador++;
    }

    sqlite3_finalize(stmt);
    printf("\n%d vehiculos encontrados\n", contador);
    return contador;
}

int listar_vehiculosEstacion(sqlite3 *db, int id_estacion){
    sqlite3_stmt *stmt;
    int contador = 0;
    sqlite3_prepare_v2(db,
        "SELECT id_vehiculo, estado, bateria FROM Vehiculo WHERE ubicacion_estacion = ?;",
        -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, id_estacion);

    while(sqlite3_step(stmt) == SQLITE_ROW){
        int id = sqlite3_column_int(stmt, 0);
        const char *estado = (const char *)sqlite3_column_text(stmt, 1);
        double bat = sqlite3_column_double(stmt, 2);
        printf("%d %s %.2f\n", id, estado ? estado : "", bat);
        contador++;
    }

    sqlite3_finalize(stmt);
    printf("\n%d vehiculos encontrados\n", contador);
    return contador;
}

int listar_usuarios(sqlite3 *db){
    sqlite3_stmt *stmt;
    int contador = 0;
    sqlite3_prepare_v2(db, "SELECT id_usuario, nombre, vehiculo_activo FROM Usuario ORDER BY id_usuario;", -1, &stmt, NULL);

    while(sqlite3_step(stmt) == SQLITE_ROW){
        int id = sqlite3_column_int(stmt, 0);
        const char *nombre = (const char *)sqlite3_column_text(stmt, 1);
        int tiene_v = sqlite3_column_int(stmt, 2);
        printf("%d %s %s\n", id, nombre ? nombre : "", tiene_v ? "Si" : "No");
        contador++;
    }

    sqlite3_finalize(stmt);
    printf("%d usuarios encontrados\n", contador);
    return contador;
}

int buscar_vehiculo(sqlite3 *db, int id, Vehiculo *resultado){
    sqlite3_stmt *stmt;
    int encontrado = 0;
    sqlite3_prepare_v2(db, "SELECT id_vehiculo, estado, ubicacion_estacion, bateria FROM Vehiculo WHERE id_vehiculo = ?;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id);

    if(sqlite3_step(stmt) == SQLITE_ROW){
        resultado->id_vehiculo = sqlite3_column_int(stmt, 0);
        const char *est = (const char *)sqlite3_column_text(stmt, 1);
        resultado->ubicacion_estacion = sqlite3_column_int(stmt, 2);
        resultado->bateria_restante = (float)sqlite3_column_double(stmt, 3);
        strcpy(resultado->estado, est ? est : "");
        encontrado = 1;
    }

    sqlite3_finalize(stmt);
    return encontrado;
}

int buscar_usuario_por_id(sqlite3 *db, int id, Usuario *resultado){
    sqlite3_stmt *stmt;
    int encontrado = 0;
    sqlite3_prepare_v2(db, "SELECT id_usuario, nombre, contrasenya, vehiculo_activo FROM Usuario WHERE id_usuario = ?;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id);

    if(sqlite3_step(stmt) == SQLITE_ROW){
        resultado->id_usuario = sqlite3_column_int(stmt, 0);
        const char *nom = (const char *)sqlite3_column_text(stmt, 1);
        const char *contra = (const char *)sqlite3_column_text(stmt, 2);
        resultado->vehiculo_activo = sqlite3_column_int(stmt, 3);
        strcpy(resultado->nombre, nom ? nom : "");
        strcpy(resultado->contrasenya, contra ? contra : "");
        encontrado = 1;
    }

    sqlite3_finalize(stmt);
    return encontrado;
}

int buscar_usuario_por_nombre(sqlite3 *db, const char *nombre, Usuario *resultado){
    sqlite3_stmt *stmt;
    int encontrado = 0;
    sqlite3_prepare_v2(db, "SELECT id_usuario, nombre, contrasenya, vehiculo_activo FROM Usuario WHERE nombre = ?;", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, nombre, -1, SQLITE_TRANSIENT);

    if(sqlite3_step(stmt) == SQLITE_ROW){
        resultado->id_usuario = sqlite3_column_int(stmt, 0);
        const char *nom = (const char *)sqlite3_column_text(stmt, 1);
        const char *contra = (const char *)sqlite3_column_text(stmt, 2);
        resultado->vehiculo_activo = sqlite3_column_int(stmt, 3);
        strcpy(resultado->nombre, nom ? nom : "");
        strcpy(resultado->contrasenya, contra ? contra : "");
        encontrado = 1;
    }

    sqlite3_finalize(stmt);
    return encontrado;
}

int cambiar_contrasenya(sqlite3 *db, int id, const char *nueva){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "UPDATE Usuario SET contrasenya = ? WHERE id_usuario = ?;", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, nueva, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}

int actualizar_estado(sqlite3 *db, int id, const char *estado){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "UPDATE Vehiculo SET estado = ? WHERE id_vehiculo = ?;", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, estado, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}

int actualizar_bateria(sqlite3 *db, int id, float bateria){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "UPDATE Vehiculo SET bateria = ? WHERE id_vehiculo = ?;", -1, &stmt, NULL);
    sqlite3_bind_double(stmt, 1, bateria);
    sqlite3_bind_int(stmt, 2, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}

int actualizar_vehiculoActivo(sqlite3 *db, int id_usuario, int id_vehiculo){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "UPDATE Usuario SET vehiculo_activo = ? WHERE id_usuario = ?;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id_vehiculo);
    sqlite3_bind_int(stmt, 2, id_usuario);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}

int poner_averia(sqlite3 *db, const Averia *a){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db,
        "INSERT INTO Averia(id_vehiculo,id_estacion,tipo,descripcion,fecha_reporte,estado)"
        " VALUES(?,?,?,?,?,?);",
        -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, a->id_vehiculo);
    sqlite3_bind_int(stmt, 2, a->id_estacion);
    sqlite3_bind_text(stmt, 3, a->tipo, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, a->descripcion, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, a->fecha, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, a->estado, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}

int listar_averiasPendientes(sqlite3 *db){
    sqlite3_stmt *stmt;
    int contador = 0;
    sqlite3_prepare_v2(db,
        "SELECT id_averia, id_vehiculo, id_estacion, tipo, descripcion, fecha_reporte "
        "FROM Averia WHERE estado = 'pendiente' ORDER BY id_averia;",
        -1, &stmt, NULL);

    while(sqlite3_step(stmt) == SQLITE_ROW){
        int id_av = sqlite3_column_int(stmt, 0);
        int id_v = sqlite3_column_int(stmt, 1);
        int id_e = sqlite3_column_int(stmt, 2);
        const char *tipo = (const char *)sqlite3_column_text(stmt, 3);
        const char *desc = (const char *)sqlite3_column_text(stmt, 4);
        const char *fecha = (const char *)sqlite3_column_text(stmt, 5);
        printf("%d %d %d %s %s %s\n", id_av, id_v, id_e, tipo ? tipo : "", desc ? desc : "", fecha ? fecha : "");
        contador++;
    }

    sqlite3_finalize(stmt);
    printf("%d averias pendientes encontradas\n", contador);
    return contador;
}

int marcar_reparada(sqlite3 *db, int id_averia){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "UPDATE Averia SET estado='reparada' WHERE id_averia = ?;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id_averia);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}

int contar_pendientes(sqlite3 *db){
    sqlite3_stmt *stmt;
    int porsierror= sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM Averia WHERE estado='pendiente';", -1, &stmt, NULL);
    if (porsierror != SQLITE_OK) {
        printf("Error: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    
    int n = 0;
    if(sqlite3_step(stmt) == SQLITE_ROW){
        n = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return n;
}

int listar_reservas(sqlite3 *db){
    sqlite3_stmt *stmt;
    int contador = 0;
    sqlite3_prepare_v2(db,
        "SELECT r.id_reserva, u.nombre, r.vehiculo_id, r.hora_inicio, r.estado "
        "FROM Reserva r JOIN Usuario u ON r.usuario_id = u.id_usuario "
        "ORDER BY r.id_reserva DESC LIMIT 30;",
        -1, &stmt, NULL);

    while(sqlite3_step(stmt) == SQLITE_ROW){
        int id_res = sqlite3_column_int(stmt, 0);
        const char *nom_user = (const char *)sqlite3_column_text(stmt, 1);
        int id_veh = sqlite3_column_int(stmt, 2);
        const char *inicio = (const char *)sqlite3_column_text(stmt, 3);
        const char *estado = (const char *)sqlite3_column_text(stmt, 4);
        printf("%d %s %d %s %s\n", id_res, nom_user ? nom_user : "", id_veh, inicio ? inicio : "", estado ? estado : "");
        contador++;
    }

    sqlite3_finalize(stmt);
    printf("%d reservas encontradas\n", contador);
    return contador;
}

int insertar_reserva(sqlite3 *db, const Reserva *r){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db,
        "INSERT INTO Reserva(usuario_id,vehiculo_id,hora_inicio,hora_fin,estado)"
        " VALUES(?,?,?,?,?);", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, r->usuario_id);
    sqlite3_bind_int(stmt, 2, r->vehiculo_id);
    sqlite3_bind_text(stmt, 3, r->hora_inicio, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, r->hora_final, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, r->estado, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}

int insertar_trayecto(sqlite3 *db, const Trayecto *t){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db,
        "INSERT INTO Trayecto(usuario_id,vehiculo_id,inicio,fin,distancia)"
        " VALUES(?,?,?,?,?);", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, t->usuario_id);
    sqlite3_bind_int(stmt, 2, t->vehiculo_id);
    sqlite3_bind_text(stmt, 3, t->inicio, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, t->fin, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 5, t->distancia);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}

int listar_trayectosUsuario(sqlite3 *db, int id_usuario){
    sqlite3_stmt *stmt;
    int contador = 0;
    sqlite3_prepare_v2(db,
        "SELECT id_trayecto, vehiculo_id, inicio, fin, distancia "
        "FROM Trayecto WHERE usuario_id = ? ORDER BY id_trayecto DESC;",
        -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, id_usuario);

    while(sqlite3_step(stmt) == SQLITE_ROW){
        int id_tray = sqlite3_column_int(stmt, 0);
        int id_v = sqlite3_column_int(stmt, 1);
        const char *t_inicio = (const char *)sqlite3_column_text(stmt, 2);
        const char *t_fin = (const char *)sqlite3_column_text(stmt, 3);
        double dist = sqlite3_column_double(stmt, 4);
        printf("%d %d %s %s %f\n", id_tray, id_v, t_inicio ? t_inicio : "", t_fin ? t_fin : "", dist);
        contador++;
    }

    sqlite3_finalize(stmt);
    printf("%d trayectos encontrados\n", contador);
    return contador;
}
void esquema_grande(sqlite3 *db){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT e.abreviacion, e.plazas, COUNT(v.id_vehiculo) FROM Estacion e "
        "LEFT JOIN Vehiculo v "
        "ON e.id_estacion=v.ubicacion_estacion "
        "GROUP BY e.id_estacion;", -1, &stmt, NULL);
   
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *abreviacion=(const char*)sqlite3_column_text(stmt,0);
            int plazas=sqlite3_column_int(stmt,1);
            int vehiculos=sqlite3_column_int(stmt,2);
            printf("Estacion %s con %d vehiculos y %d plazas  \n",abreviacion,plazas,vehiculos);

    }
    sqlite3_finalize(stmt);
}

void esquema_pequenyo(sqlite3 *db){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT e.abreviacion, e.plazas, COUNT(v.id_vehiculo) FROM Estacion e "
        "LEFT JOIN Vehiculo v "
        "ON e.id_estacion=v.ubicacion_estacion "
        "GROUP BY e.id_estacion;", -1, &stmt, NULL);
   
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *abreviacion=(const char*)sqlite3_column_text(stmt,0);
            int plazas=sqlite3_column_int(stmt,1);
            int vehiculos=sqlite3_column_int(stmt,2);
            printf("[%s] %d %d\n",abreviacion,plazas,vehiculos);

    }
    sqlite3_finalize(stmt);
}

void db_estadisticas(sqlite3 *db){
    sqlite3_stmt *s;
    int n;

    printf("\n  +------------------------------+\n");
    printf("  |   ESTADISTICAS DEL SISTEMA   |\n");
    printf("  +------------------------------+\n\n");

    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM Usuario;", -1, &s, NULL);
    n = 0;
    if(sqlite3_step(s) == SQLITE_ROW) n = sqlite3_column_int(s, 0);
    sqlite3_finalize(s);
    printf("  Usuarios registrados : %d\n", n);

    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM Vehiculo;", -1, &s, NULL);
    n = 0;
    if(sqlite3_step(s) == SQLITE_ROW) n = sqlite3_column_int(s, 0);
    sqlite3_finalize(s);
    printf("  Vehiculos totales    : %d\n", n);

    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM Vehiculo WHERE estado='disponible';", -1, &s, NULL);
    n = 0;
    if(sqlite3_step(s) == SQLITE_ROW) n = sqlite3_column_int(s, 0);
    sqlite3_finalize(s);
    printf("    - Disponibles      : %d\n", n);

    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM Vehiculo WHERE estado='en_uso';", -1, &s, NULL);
    n = 0;
    if(sqlite3_step(s) == SQLITE_ROW) n = sqlite3_column_int(s, 0);
    sqlite3_finalize(s);
    printf("    - En uso           : %d\n", n);

    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM Vehiculo WHERE estado='averiado';", -1, &s, NULL);
    n = 0;
    if(sqlite3_step(s) == SQLITE_ROW) n = sqlite3_column_int(s, 0);
    sqlite3_finalize(s);
    printf("    - Averiados        : %d\n", n);

    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM Averia WHERE estado='pendiente';", -1, &s, NULL);
    n = 0;
    if(sqlite3_step(s) == SQLITE_ROW) n = sqlite3_column_int(s, 0);
    sqlite3_finalize(s);
    printf("  Averias pendientes   : %d\n", n);

    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM Reserva WHERE estado='activa';", -1, &s, NULL);
    n = 0;
    if(sqlite3_step(s) == SQLITE_ROW) n = sqlite3_column_int(s, 0);
    sqlite3_finalize(s);
    printf("  Reservas activas     : %d\n", n);

    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM Trayecto;", -1, &s, NULL);
    n = 0;
    if(sqlite3_step(s) == SQLITE_ROW) n = sqlite3_column_int(s, 0);
    sqlite3_finalize(s);
    printf("  Trayectos realizados : %d\n", n);

    sqlite3_prepare_v2(db,
        "SELECT e.nombre, COUNT(*) as c FROM Vehiculo v "
        "JOIN Estacion e ON v.ubicacion_estacion=e.id_estacion "
        "WHERE v.estado='disponible' GROUP BY v.ubicacion_estacion "
        "ORDER BY c DESC LIMIT 1;", -1, &s, NULL);

    if(sqlite3_step(s) == SQLITE_ROW){
        printf("  Estacion con mas disponibles: %s (%d)\n",
               sqlite3_column_text(s, 0),
               sqlite3_column_int(s, 1));
    }

    sqlite3_finalize(s);
    printf("\n");
}


// cambios en admin.c
#include "admin.h"
#include "db.h"
#include "logic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int admin_login(const Config *cfg){
    char usuario[MAX_VALOR], clave[MAX_VALOR];
    int intentos = 3;

    printf("\n +================================+\n");
    printf(" | SISTEMA DE GESTION EUSKOKAR    |\n");
    printf(" | ADMINISTRADOR LOCAL            |\n");
    printf(" +================================+\n\n");

    while(intentos > 0){
        intentos--;
        leer_cadena(" Usuario: ", usuario, MAX_VALOR);
        leer_cadena(" Clave  : ", clave, MAX_VALOR);

        if(strcmp(usuario, cfg->admin_usuario) == 0 &&
           strcmp(clave, cfg->admin_clave) == 0){
            printf("Bienvenido, %s\n", usuario);
            return 1;
        }

        printf("Datos incorrectos: intentos restantes %i\n", intentos);
    }

    printf("Demasiados intentos fallidos. Saliendo.\n");
    return 0;
}

static void menu_vehiculos(sqlite3 *db, const Config *cfg){
    int op = -1;

    while(op != 0){
        printf("\n +---------------------+\n");
        printf(" | GESTION VEHICULOS   |\n");       
        printf(" +---------------------+\n");
        printf(" 1. Listar todos los vehiculos\n");
        printf(" 2. Vehiculos de una estacion\n");
        printf(" 3. Cambiar estado de vehiculo\n");
        printf(" 4. Actualizar autonomia (bateria)\n");
        printf(" 0. Volver\n");

        op = leer_entero(" Opcion: ", 0, 4);

        if(op == 1){
            listar_vehiculos(db);  
        }else if(op == 2){
            int id = leer_entero(" ID de estacion: ", 1, 9999);
            listar_vehiculosEstacion(db, id);  
        }else if(op == 3){
            logic_cambiar_estado_vehiculo(db, cfg);
        }else if(op == 4){
            int id = leer_entero(" ID del vehiculo: ", 1, 9999);
            Vehiculo v;

            if(buscar_vehiculo(db, id, &v)){  
                printf("Autonomia actual: %.1f%%\n", v.bateria_restante);
                int nueva = leer_entero(" Nueva autonomia (0-100): ", 0, 100);
                actualizar_bateria(db, id, (float)nueva);  
                printf("Autonomia actualizada\n");

                char msg[128];
                snprintf(msg, sizeof(msg),
                         "VEHICULO %i autonomia actualizada a %d\n",
                         id, nueva);
                log_escribir(cfg, msg);
            }else{
                printf("Vehiculo no encontrado\n");
            }
        }
    }
}

static void menu_usuarios(sqlite3 *db){
    int op = -1;

    while(op != 0){
        printf("\n +---------------------+\n");
        printf(" | GESTION USUARIOS    |\n");
        printf(" +---------------------+\n");
        printf(" 1. Listar todos los usuarios\n");
        printf(" 2. Buscar usuario por ID\n");
        printf(" 3. Cambiar contrasena de usuario\n");
        printf(" 0. Volver\n");

        op = leer_entero(" Opcion: ", 0, 3);

        if(op == 1){
            listar_usuarios(db);  
        }else if(op == 2){
            int id = leer_entero(" ID del usuario: ", 1, 99999);
            Usuario u;

            if(buscar_usuario_por_id(db, id, &u)){  
                printf(" ID: %d\n", u.id_usuario);
                printf(" Nombre: %s\n", u.nombre);
                printf(" Veh.activo: %s\n", u.vehiculo_activo ? "Si" : "No");
            }else{
                printf("Usuario no encontrado\n");
            }
        }else if(op == 3){
            logic_cambiar_contrasena_usuario(db);
        }
    }
}

static void menu_averias(sqlite3 *db, const Config *cfg){
    int op = -1;

    while(op != 0){
        int pendientes = contar_pendientes(db);  

        printf("\n +-------------------+\n");
        printf(" | GESTION AVERIAS   |\n");
        printf(" +-------------------+\n");
        printf(" Averias pendientes: %d\n\n", pendientes);
        printf(" 1. Ver averias pendientes\n");
        printf(" 2. Registrar nueva averia\n");
        printf(" 3. Marcar averia como reparada\n");
        printf(" 0. Volver\n");

        op = leer_entero(" Opcion: ", 0, 3);

        if(op == 1){
            listar_averiasPendientes(db);  
        }else if(op == 2){
            logic_registrar_averia(db, cfg);
        }else if(op == 3){
            listar_averiasPendientes(db);  
            int id = leer_entero(" ID de la averia a marcar reparada: ", 1, 99999);

            if(marcar_reparada(db, id)){  
                printf("Averia %i marcada como reparada\n", id);

                char msg[128];
                snprintf(msg, sizeof(msg), "AVERIA %i marcada como reparada\n", id);
                log_escribir(cfg, msg);
            }else{
                printf("No se encontro la averia o ya estaba reparada\n");
            }
        }
    }
}

static void menu_estaciones(sqlite3 *db){
    int op = -1;

    while(op != 0){
        printf("\n +--------------------+\n");
        printf(" | GESTION ESTACIONES |\n");
        printf(" +--------------------+\n");
        printf(" 1. Listar todas las estaciones\n");
        printf(" 2. Ver vehiculos de una estacion\n");
        printf(" 0. Volver\n");

        op = leer_entero(" Opcion: ", 0, 2);

        if(op == 1){
            listar_estaciones(db);  
        }else if(op == 2){
            int id = leer_entero(" ID de estacion: ", 1, 9999);
            listar_vehiculosEstacion(db, id);  
        }
    }
}

void admin_menu(sqlite3 *db, const Config *cfg){
    log_escribir(cfg, "SESION iniciada por administrador");
    int op = -1;

    while(op != 0){
        printf("\n +========================+\n");
        printf(" | MENU PRINCIPAL ADMIN   |\n");
        printf(" +========================+\n");
        printf(" 1. Gestion de estaciones\n");
        printf(" 2. Gestion de vehiculos\n");
        printf(" 3. Gestion de usuarios\n");
        printf(" 4. Gestion de averias\n");
        printf(" 5. Estadisticas del sistema\n");
        printf(" 6. Ver log del sistema\n");
        printf(" 7. Ver reservas\n");
        printf(" 8. Ver esquema completo\n");
        printf(" 9. Ver esquema reducido\n");
        printf(" 0. Salir\n");

        op = leer_entero(" Opcion: ", 0, 9);

        if(op == 1){
            menu_estaciones(db);
        }else if(op == 2){
            menu_vehiculos(db, cfg);
        }else if(op == 3){
            menu_usuarios(db);
        }else if(op == 4){
            menu_averias(db, cfg);
        }else if(op == 5){
            db_estadisticas(db);
        }else if(op == 6){
            logic_mostrar_log(cfg);
        }else if(op == 7){
            listar_reservas(db);  
        }else if(op == 8){
            esquema_grande(db);  
        }
        else if(op == 9){
            esquema_pequenyo(db);  
        }
    }

    log_escribir(cfg, "SESION cerrada por administrador");
    printf("\n Sesion cerrada. Hasta pronto.\n");
}