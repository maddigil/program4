#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


sqlite3 *abrir_baseDatos(char *ruta){
    sqlite3 *db;
    if (sqlite3_open(ruta, &db) != SQLITE_OK)
    { // esto va a abrir el fichero y si no existe lo crea
        printf("Ha habido un error al abrirlo '%s': %s\n", ruta);
        return NULL;
    }
    return db;
}
void cerrar_baseDatos(sqlite3 *db){
    sqlite3_close(db);
}
int crearTablas(sqlite3 *db)
{
    char *sql =
        // estaciones de carga
        "CREATE TABLE IF NOT EXISTS Estacion ("
        "  id_estacion  INTEGER PRIMARY KEY,"
        "  nombre       TEXT NOT NULL,"
        "  abreviacion  TEXT,"
        "  plazas       INTEGER DEFAULT 0"
        ");"
        // vehiculos electricos
        "CREATE TABLE IF NOT EXISTS Vehiculo ("
        "  id_vehiculo          INTEGER PRIMARY KEY,"
        "  estado               TEXT DEFAULT 'disponible',"
        "  ubicacion_estacion   INTEGER REFERENCES Estacion(id_estacion),"
        "  bateria              REAL DEFAULT 100.0" // esto es el % de la bateria
        ");"
        // usuarios del sistema
        "CREATE TABLE IF NOT EXISTS Usuario ("
        "  id_usuario      INTEGER PRIMARY KEY,"
        "  nombre          TEXT NOT NULL,"
        "  contrasenya      TEXT NOT NULL DEFAULT '1234'," // contrasenya por defecto
        "  vehiculo_activo INTEGER DEFAULT 0"              // da 0 si el vehiculo no esta activo
        ");"
        // averias reportadas
        "CREATE TABLE IF NOT EXISTS Averia ("
        "  id_averia      INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  id_vehiculo    INTEGER DEFAULT 0," // da 0 si la avería es de estación
        "  id_estacion    INTEGER DEFAULT 1," // y 1 si es de estacion  
        "  tipo           TEXT,"              // el tipo de averia tipo: mecanica, de bateria, en la carga...
        "  descripcion    TEXT,"
        "  fecha_reporte  TEXT,"
        "  estado         TEXT DEFAULT 'pendiente'" // si todavia esta pendiente o esta ya arreglado
        ");"
        // reservas de vehiculos
        "CREATE TABLE IF NOT EXISTS Reserva ("
        "  id_reserva   INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  usuario_id   INTEGER REFERENCES Usuario(id_usuario),"
        "  vehiculo_id  INTEGER REFERENCES Vehiculo(id_vehiculo),"
        "  hora_inicio  TEXT,"
        "  hora_fin     TEXT,"
        "  estado       TEXT DEFAULT 'activa'" // los estados pueden ser: activa,cancelada o finalizada
        ");"
        // trayectos realizados
        "CREATE TABLE IF NOT EXISTS Trayecto ("
        "  id_trayecto  INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  usuario_id   INTEGER REFERENCES Usuario(id_usuario),"
        "  vehiculo_id  INTEGER REFERENCES Vehiculo(id_vehiculo),"
        "  inicio       TEXT,"            // fecha y hora de inicio del trayecto
        "  fin          TEXT,"            // lo mismo pero del final
        "  distancia    REAL DEFAULT 0.0" // los kms que has hecho
        ");";


    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK)//v esto es que no ha habido error
    {
        printf("Ha habido un error creando tablas\n");
        return 0;
    }
    return 1;
}
static void limpiarCsv(char *string){
    for(int i=0;string[i] != '\0';i++){
        if(string[i]=='\r'|| string[i]=='\n'){
            string[i]='\0';
        }
    }
}
static char *SaltarBom(char *texto){
    if (texto[0] == 0xEF && texto[1] == 0xBB && texto[2] == 0xBF)
    {
        return texto + 3; // saltar los 3 bytes del BOM
    }
    return texto; // si no hay BOM te da el string como esta
}
int cargar_estaciones(sqlite3 *db, const char *csv){
    FILE *fd=fopen(csv,"r");
    if(fd==NULL){
        perror("Error");
        exit(1);
    }
    char linea[256];
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "INSERT INTO Estacion VALUES(?,?,?,?);", -1, &stmt,NULL);
    int contador=0;
    while(fgets(linea, sizeof(linea), fd)){
        limpiarCsv(linea);
        int id;
        int plazas;
        char nombre[100];
        char abrev[20];
        sscanf(linea, "%d,%s,%s,%d\n", &id, nombre, abrev, &plazas);
        sqlite3_bind_int(stmt, 1, id);
            sqlite3_bind_text(stmt, 2, nombre, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, abrev, -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 4, plazas);


            sqlite3_step(stmt);
            sqlite3_reset(stmt);
            contador++;


    }
    sqlite3_finalize(stmt);
    fclose(fd);
    printf("Se han cargado %d estaciones.",contador);
    return contador;
}
int cargar_usuarios(sqlite3 *db, const char *csv){
    FILE *fd=fopen(csv,"r");
    if(fd==NULL){
        perror("Error");
        exit(1);
    }
    char linea[256];
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db,  "INSERT OR IGNORE INTO Usuario VALUES(?,?, '1234',0);", -1, &stmt,NULL);
    int contador=0;
    while(fgets(linea, sizeof(linea), fd)){
        limpiarCsv(linea);
        int id;
        char nombre[100];
        sscanf(linea, "%d,%s\n", &id, nombre);
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_bind_text(stmt, 2, nombre, -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);


        sqlite3_step(stmt);
        sqlite3_reset(stmt);
        contador++;


    }
    sqlite3_finalize(stmt);
    fclose(fd);
    printf("Se han cargado %d usuarios.",contador);
    return contador;
}
int cargar_vehiculos(sqlite3 *db, const char *csv){
    FILE *fd=fopen(csv,"r");
    if(fd==NULL){
        perror("Error");
        exit(1);
    }
    char linea[256];
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO Vehiculo VALUES(?,?,?,?);", -1, &stmt,NULL);
    int contador=0;
    while(fgets(linea, sizeof(linea), fd)){
        limpiarCsv(linea);
        int id;
        int id_estacion;
        char estado[50];
        float bateria;


        sscanf(linea, "%d,%s,%d,%f\n", &id, estado, &id_estacion, &bateria);
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_bind_text(stmt, 2, estado, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, id_estacion);
        sqlite3_bind_double(stmt, 4, bateria);


        sqlite3_step(stmt);
        sqlite3_reset(stmt);
        contador++;


    }
    sqlite3_finalize(stmt);
    fclose(fd);
    printf("Se han cargado %d vehiculos.",contador);
    return contador;
}
int listar_estaciones(sqlite3 *db){
    sqlite3_stmt *stmt;
    int contador = 0;
    sqlite3_prepare_v2(db, "SELECT id_estacion, abreviacion, nombre,plazas FROM Estacion;", -1, &stmt, NULL);
   
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *abrev =sqlite3_column_text(stmt, 1);
        const unsigned char *nom = sqlite3_column_text(stmt, 2);
        int plazas = sqlite3_column_int(stmt, 3);


        printf("%d %s %s %d\n", id, abrev, nom, plazas);
        contador++;
    }
    sqlite3_finalize(stmt);
    printf("%d estaciones encontradas(si es 0 es que no hay estaciones registradas)\n", contador);
    return contador;
}
int listar_vehiculos(sqlite3 *db){
    sqlite3_stmt *stmt;
    int contador = 0;
    sqlite3_prepare_v2(db,"SELECT v.id_vehiculo, v.estado, v.bateria, e.abreviacion "
                      "FROM Vehiculo v LEFT JOIN Estacion e ON v.ubicacion_estacion = e.id_estacion;" , -1, &stmt, NULL);
   
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const char *estado = (const char *)sqlite3_column_text(stmt, 1);
        double bat = sqlite3_column_double(stmt, 2);
        const char *estacion = (const char *)sqlite3_column_text(stmt, 3);


        char estacion_nom[20];
        if (estacion == NULL)
        {
            sprintf(estacion_nom, "-"); //esto es que si no hay estacion estacion_nom sera -
        }
        else
        {
            sprintf(estacion_nom, "%s", estacion);//si no la estacion
        }
        printf("%d %s %.2f %s\n", id, estado, bat, estacion_nom);
        contador++;
    }
    sqlite3_finalize(stmt);
    printf("%d vehiculos encontradas(si es 0 es que no hay vehiculos registradas)\n", contador);
    return contador;
}
int listar_vehiculosEstacion(sqlite3 *db, int id_estacion){
    sqlite3_stmt *stmt;
    int contador = 0;
    sqlite3_prepare_v2(db,"SELECT id_vehiculo, estado, bateria FROM Vehiculo WHERE ubicacion_estacion = ?;", -1, &stmt, NULL);
   
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const char *estado = (const char *)sqlite3_column_text(stmt, 1);
        double bat = sqlite3_column_double(stmt, 2);


        printf("%d %s %.2f\n", id, estado, bat);
        contador++;
    }
    sqlite3_finalize(stmt);
    printf("%d vehiculos encontradas(si es 0 es que no hay vehiculos registradas)\n", contador);
    return contador;
}
int listar_usuarios(sqlite3 *db){
    sqlite3_stmt *stmt;
    int contador = 0;
    sqlite3_prepare_v2(db, "SELECT id_usuario, nombre, vehiculo_activo FROM Usuario ORDER BY id_usuario;", -1, &stmt, NULL);
   
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const char *nombre = (const char *)sqlite3_column_text(stmt, 1);
        int tiene_v = sqlite3_column_int(stmt, 2);


        char estado_vehiculo[5];
        if (tiene_v == 0)
        {
            sprintf(estado_vehiculo, "No");
        }
        else
        {
            sprintf(estado_vehiculo, "Si");
        }


        printf("%d %s %s\n", id, nombre, estado_vehiculo);
        contador++;
    }
    sqlite3_finalize(stmt);
    printf("%d Usuarios encontrados(si es 0 es que no hay usuarios registrados)\n", contador);
    return contador;
}
int buscar_vehiculo(sqlite3 *db, int id, Vehiculo *resultado){
    sqlite3_stmt *stmt;
    int encontrado = 0;
    sqlite3_prepare_v2(db, "SELECT id_vehiculo, estado,ubicacion_estacion, bateria FROM Vehiculo WHERE id_vehiculo =?;", -1, &stmt, NULL);
    qlite3_bind_int(stmt, 1, id);//esto es para vincularlo
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id_v = sqlite3_column_int(stmt, 0);
        const char *est = (const char *)sqlite3_column_text(stmt, 1);
        int id_est = sqlite3_column_int(stmt, 2);
        float bat = (float)sqlite3_column_double(stmt, 3);


        resultado->id_vehiculo = id_v; //esto es que si lo ha encontrado que lo asigna
        resultado->ubicacion_estacion = id_est;
        resultado->bateria = bat;


        strcpy(resultado->estado, est);


        encontrado = 1;
    }
    sqlite3_finalize(stmt);
    return encontrado;
}
int buscar_usuario_por_id(sqlite3 *db, int id, Usuario *resultado){
    sqlite3_stmt *stmt;
    int encontrado = 0;
    sqlite3_prepare_v2(db, "SELECT id_usuario, nombre, contrasenya, vehiculo_activo FROM Usuario WHERE id_usuario = ?;", -1, &stmt, NULL);
    qlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id_u = sqlite3_column_int(stmt, 0);
        const char *nom = (const char *)sqlite3_column_text(stmt, 1);
        const char *contra = (const char *)sqlite3_column_text(stmt, 2);
        int activa = sqlite3_column_int(stmt, 3);


        resultado->id_usuario = id_u;
        resultado->vehiculo_activo = activa;


        strcpy(resultado->nombre, nom);
        strcpy(resultado->contrasenya, contra);


        encontrado = 1;
    }
    sqlite3_finalize(stmt);
    return encontrado;
}
int buscar_usuario_por_nombre(sqlite3 *db, const char *nombre, Usuario *resultado){
    sqlite3_stmt *stmt;
    int encontrado = 0;
    sqlite3_prepare_v2(db, "SELECT id_usuario, nombre, contrasenya, vehiculo_activo FROM Usuario WHERE nombre = ?;", -1, &stmt, NULL);
    qlite3_bind_text(stmt, 1, nombre);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id_u = sqlite3_column_int(stmt, 0);
        const char *nom = (const char *)sqlite3_column_text(stmt, 1);
        const char *contra = (const char *)sqlite3_column_text(stmt, 2);
        int activa = sqlite3_column_int(stmt, 3);


        resultado->id_usuario = id_u;
        resultado->vehiculo_activo = activa;
        strcpy(resultado->nombre, nom);
        strcpy(resultado->contrasenya, contra);


        encontrado = 1;
    }
    sqlite3_finalize(stmt);
    return encontrado;
}
int cambiar_contrasenya(sqlite3 *db, int id, const char *nueva){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "UPDATE Usuario SET contrasenya=? WHERE id_usuario=?;", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, nueva, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}
int actualizar_estado(sqlite3 *db, int id, const char *estado){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "UPDATE Vehiculo SET estado=? WHERE id_vehiculo=?;", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, estado, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}
int actualizar_bateria(sqlite3 *db, int id, float bateria){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "UPDATE Vehiculo SET bateria=? WHERE id_vehiculo=?;", -1, &stmt, NULL);
    sqlite3_bind_double(stmt, 1, bateria);
    sqlite3_bind_int(stmt, 2, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db);


}
int actualizar_vehiculoActivo(sqlite3 *db, int id_usuario, int id_vehiculo){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "UPDATE Usuario SET vehiculo_activo=? WHERE id_usuario=?;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id_vehiculo);
    sqlite3_bind_int(stmt, 2, id_usuario);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}
int poner_averia(sqlite3 *db, const Averia *a){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "INSERT INTO Averia(id_vehiculo,id_estacion,tipo,descripcion,fecha_reporte,estado)"
                       " VALUES(?,?,?,?,?,?);", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, a->id_vehiculo);
    sqlite3_bind_int(stmt, 2, a->id_estacion);
    sqlite3_bind_text(stmt, 3, a->tipo, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, a->descripcion, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, a->fecha_reporte, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, a->estado, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}
int listar_averiasPendientes(sqlite3 *db){
    sqlite3_stmt *stmt;
    int contador = 0;
    sqlite3_prepare_v2(db, "SELECT id_averia, id_vehiculo, id_estacion, tipo, descripcion, fecha_reporte "
                      "FROM Averia WHERE estado = 'pendiente' ORDER BY id_averia;", -1, &stmt, NULL);
   
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id_av = sqlite3_column_int(stmt, 0);
        int id_v = sqlite3_column_int(stmt, 1);
        int id_e = sqlite3_column_int(stmt, 2);
        const char *tipo = (const char *)sqlite3_column_text(stmt, 3);
        const char *desc = (const char *)sqlite3_column_text(stmt, 4);
        const char *fecha = (const char *)sqlite3_column_text(stmt, 5);
        printf("%d %d %d %s %s %s\n", id_av, id_v, id_e, tipo, desc, fecha);
        contador++;
    }
    sqlite3_finalize(stmt);
    printf("%d averias pendientes encontradas(si es 0 es que no hay averias pendientes registradas)\n", contador);
    return contador;
}
int marcar_reparada(sqlite3 *db, int id_averia){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "UPDATE Averia SET estado='reparada' WHERE id_averia=?;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id_averia);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}
int contar_pendientes(sqlite3 *db){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db,"SELECT COUNT(*) FROM Averia WHERE estado='pendiente';", -1, &stmt, NULL);
    int n = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        n = sqlite3_column_int(stmt, 0);//es la primera columna del resultado del count
    sqlite3_finalize(stmt);
    return n;
}
int listar_reservas(sqlite3 *db){
  sqlite3_stmt *stmt;
    int contador = 0;
    sqlite3_prepare_v2(db, "SELECT r.id_reserva, u.nombre, r.vehiculo_id, r.hora_inicio, r.estado "
                      "FROM Reserva r JOIN Usuario u ON r.usuario_id = u.id_usuario "
                      "ORDER BY r.id_reserva DESC LIMIT 30;", -1, &stmt, NULL);
   
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id_res = sqlite3_column_int(stmt, 0);
        const char *nom_user = (const char *)sqlite3_column_text(stmt, 1);
        int id_veh = sqlite3_column_int(stmt, 2);
        const char *inicio = (const char *)sqlite3_column_text(stmt, 3);
        const char *estado = (const char *)sqlite3_column_text(stmt, 4);


        printf("%d %s %d %s %s\n", id_res, nom_user, id_veh, inicio, estado);
        contador++;
    }
    sqlite3_finalize(stmt);
    printf("%d reservas encontradas(si es 0 es que no hay reservas registradas)\n", contador);
    return contador;
}
int insertar_reserva(sqlite3 *db, const Reserva *r){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "INSERT INTO Reserva(usuario_id,vehiculo_id,hora_inicio,hora_fin,estado)"
                    " VALUES(?,?,?,?,?);", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, r->usuario_id);
    sqlite3_bind_int(stmt, 2, r->vehiculo_id);
    sqlite3_bind_text(stmt, 3, r->hora_inicio, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, r->hora_fin, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, r->estado, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db);
}
int insertar_trayecto(sqlite3 *db, const Trayecto *t){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db,"INSERT INTO Trayecto(usuario_id,vehiculo_id,inicio,fin,distancia)"
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
    sqlite3_prepare_v2(db, "SELECT id_trayecto, vehiculo_id, inicio, fin, distancia "
                      "FROM Trayecto WHERE usuario_id = ? ORDER BY id_trayecto DESC;", -1, &stmt, NULL);
   
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id_tray = sqlite3_column_int(stmt, 0);
        int id_v = sqlite3_column_int(stmt, 1);
        const char *t_inicio = (const char *)sqlite3_column_text(stmt, 2);
        const char *t_fin = (const char *)sqlite3_column_text(stmt, 3);
        double dist = sqlite3_column_double(stmt, 4);


        printf("%d %d %s %s %f\n", id_tray, id_v, t_inicio, t_fin, dist);
        contador++;
    }
    sqlite3_finalize(stmt);
    printf("%d trayectos encontrados(si es 0 es que no hay trayectos registrados)\n", contador);
    return contador;
}
