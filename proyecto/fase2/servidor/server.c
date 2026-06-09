#include "server.h"
#include "db.h"
#include "logic.h"
#include "protocolo.h"

void expirar_reservas_caducadas(sqlite3 *db);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <unistd.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
#endif

static void net_enviar(int fd, const char *msg) {
    char buf[TAM_BUF];
    int n = snprintf(buf, sizeof(buf), "%s\n", msg);
    send(fd, buf, n, 0);
}

static void net_ok(int fd, const char *dato) {
    char buf[TAM_BUF];
    if (dato && dato[0] != '\0')
        snprintf(buf, sizeof(buf), "OK %s", dato);
    else
        snprintf(buf, sizeof(buf), "OK");
    net_enviar(fd, buf);
}

static void net_error(int fd, const char *msg) {
    char buf[TAM_BUF];
    snprintf(buf, sizeof(buf), "ERROR %s", msg);
    net_enviar(fd, buf);
}


static void cmd_listar_estaciones(int fd, sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT e.id_estacion, e.abreviacion, e.nombre, e.plazas, "
        "       COUNT(v.id_vehiculo) AS total_vehiculos, "
        "       COUNT(CASE WHEN v.estado='disponible' THEN 1 END) AS disponibles "
        "FROM Estacion e "
        "LEFT JOIN Vehiculo v ON e.id_estacion = v.ubicacion_estacion "
        "GROUP BY e.id_estacion "
        "ORDER BY e.id_estacion;";


    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        net_error(fd, "Error interno al listar estaciones");
        return;
    }

    net_enviar(fd, RESP_OK);

    char linea[256];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int         id   = sqlite3_column_int(stmt, 0);
        const char *ab   = (const char *)sqlite3_column_text(stmt, 1);
        const char *nom  = (const char *)sqlite3_column_text(stmt, 2);
        int         plaz = sqlite3_column_int(stmt, 3);
        int         disp = sqlite3_column_int(stmt, 4);
        snprintf(linea, sizeof(linea), "%d|%s|%s|%d|%d",
                 id, ab ? ab : "?", nom ? nom : "?", plaz, disp);
        net_enviar(fd, linea);
    }

    net_enviar(fd, RESP_FIN);
    sqlite3_finalize(stmt);
}


static void cmd_vehiculos_estacion(int fd, sqlite3 *db, int id_estacion) {
    /* Expirar reservas caducadas antes de mostrar estados */
    sqlite3_stmt *stmt;
    expirar_reservas_caducadas(db);
    const char *sql =
        "SELECT id_vehiculo, estado, bateria "
        "FROM Vehiculo WHERE ubicacion_estacion = ? "
        "ORDER BY id_vehiculo;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        net_error(fd, "Error interno al listar vehiculos");
        return;
    }

    sqlite3_bind_int(stmt, 1, id_estacion);
    net_enviar(fd, RESP_OK);

    char linea[128];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char *estado_db = (const char *)sqlite3_column_text(stmt, 1);
        double bat = sqlite3_column_double(stmt, 2);

        char estado_envio[32];
        strncpy(estado_envio, estado_db ? estado_db : "desconocido", sizeof(estado_envio) - 1);
        estado_envio[sizeof(estado_envio) - 1] = '\0';

        int usuario_reserva = reserva_activa_vehiculo(db, id);
        if (usuario_reserva != 0) {
            strncpy(estado_envio, "reservado", sizeof(estado_envio) - 1);
            estado_envio[sizeof(estado_envio) - 1] = '\0';
        }

        snprintf(linea, sizeof(linea), "%d|%s|%.1f", id, estado_envio, bat);
        net_enviar(fd, linea);
    }

    net_enviar(fd, RESP_FIN);
    sqlite3_finalize(stmt);
}

static void cmd_listar_vehiculos(int fd, sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT v.id_vehiculo, v.estado, v.bateria, e.abreviacion "
        "FROM Vehiculo v "
        "LEFT JOIN Estacion e ON v.ubicacion_estacion = e.id_estacion "
        "ORDER BY v.id_vehiculo;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    net_enviar(fd, RESP_OK);

    char linea[128];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int         id  = sqlite3_column_int(stmt, 0);
        const char *est = (const char *)sqlite3_column_text(stmt, 1);
        double      bat = sqlite3_column_double(stmt, 2);
        const char *ab  = (const char *)sqlite3_column_text(stmt, 3);
        snprintf(linea, sizeof(linea), "%d|%s|%.1f|%s",
                 id, est ? est : "?", bat, ab ? ab : "-");
        net_enviar(fd, linea);
    }

    net_enviar(fd, RESP_FIN);
    sqlite3_finalize(stmt);
}

static int cmd_login(int fd, sqlite3 *db, const char *params,
                     int *id_usuario_out) {
    char nombre[100] = {0}, clave[100] = {0};
    const char *sep = strrchr(params, '|');
    if (sep) {
        int len = (int)(sep - params);
        if (len >= (int)sizeof(nombre)) len = (int)sizeof(nombre) - 1;
        strncpy(nombre, params, len);
        nombre[len] = '\0';
        strncpy(clave, sep + 1, sizeof(clave) - 1);
    } else {
        sscanf(params, "%99s %99s", nombre, clave);
    }

    Usuario u;
    if (!buscar_usuario_por_nombre(db, nombre, &u)) {
        net_error(fd, "Usuario no encontrado");
        return 0;
    }

    if (strcmp(u.contrasenya, clave) != 0) {
        net_error(fd, "Contrasena incorrecta");
        return 0;
    }

    *id_usuario_out = u.id_usuario;
    char resp[256];
    snprintf(resp, sizeof(resp), "%d %s", u.id_usuario, u.nombre);
    net_ok(fd, resp);
    return 1;
}


static void cmd_reservar(int fd, sqlite3 *db, const Config *cfg,
                          int id_usuario, int id_vehiculo) {
    expirar_reservas_caducadas(db);
    Usuario u;
    if (!buscar_usuario_por_id(db, id_usuario, &u)) {
        net_error(fd, "Usuario no encontrado");
        return;
    }
    if (u.vehiculo_activo != 0) {
        net_error(fd, "Ya tienes un vehiculo activo. Finaliza el trayecto primero.");
        return;
    }

    Vehiculo v;
    if (!buscar_vehiculo(db, id_vehiculo, &v)) {
        net_error(fd, "Vehiculo no encontrado");
        return;
    }
    if (strcmp(v.estado, "disponible") != 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Vehiculo no disponible (estado: %s)", v.estado);
        net_error(fd, msg);
        return;
    }

    int reservante = reserva_activa_vehiculo(db, id_vehiculo);
    if (reservante != 0 && reservante != id_usuario) {
        net_error(fd, "Ese vehiculo ya tiene una reserva activa de otro usuario");
        return;
    }
    if (reservante == id_usuario) {
        net_ok(fd, "Ya tienes una reserva activa para ese vehiculo");
        return;
    }

    char ahora[32];
    fecha_ahora(ahora, sizeof(ahora));
    Reserva r;
    memset(&r, 0, sizeof(r));
    r.usuario_id  = id_usuario;
    r.vehiculo_id = id_vehiculo;
    strncpy(r.hora_inicio, ahora, sizeof(r.hora_inicio) - 1);
    strncpy(r.hora_final,  ahora, sizeof(r.hora_final)  - 1);
    strncpy(r.estado, "activa", sizeof(r.estado) - 1);

    if (!insertar_reserva(db, &r)) {
        net_error(fd, "Error al crear la reserva en la base de datos");
        return;
    }

    char logmsg[128];
    snprintf(logmsg, sizeof(logmsg),
             "RESERVA usuario %d vehiculo %d", id_usuario, id_vehiculo);
    log_escribir(cfg, logmsg);

    char resp[64];
    snprintf(resp, sizeof(resp), "Reserva creada para vehiculo %d", id_vehiculo);
    net_ok(fd, resp);
}


static void cmd_usar_vehiculo(int fd, sqlite3 *db, const Config *cfg,
                               int id_usuario, int id_vehiculo) {
    Usuario u;
    if (!buscar_usuario_por_id(db, id_usuario, &u)) {
        net_error(fd, "Usuario no encontrado");
        return;
    }
    if (u.vehiculo_activo != 0) {
        net_error(fd, "Ya tienes un vehiculo en uso");
        return;
    }

    Vehiculo v;
    if (!buscar_vehiculo(db, id_vehiculo, &v)) {
        net_error(fd, "Vehiculo no encontrado");
        return;
    }
    if (strcmp(v.estado, "disponible") != 0) {
        net_error(fd, "El vehiculo no esta disponible");
        return;
    }

    cancelar_reservas_vehiculo(db, id_vehiculo);

    char ahora[32];
    fecha_ahora(ahora, sizeof(ahora));
    Trayecto t;
    memset(&t, 0, sizeof(t));
    t.usuario_id       = id_usuario;
    t.vehiculo_id      = id_vehiculo;
    t.estacion_origen  = v.ubicacion_estacion;  /* Guardamos de dónde sale */
    t.estacion_destino = 0;                     /* Se rellena al finalizar  */
    strncpy(t.inicio, ahora, sizeof(t.inicio) - 1);
    t.distancia = 0.0;

    insertar_trayecto(db, &t);

    sqlite3_stmt *s;
    int id_tray = 0;
    sqlite3_prepare_v2(db, "SELECT MAX(id_trayecto) FROM Trayecto;", -1, &s, NULL);
    if (sqlite3_step(s) == SQLITE_ROW) id_tray = sqlite3_column_int(s, 0);
    sqlite3_finalize(s);

    actualizar_estado(db, id_vehiculo, "en_uso");
    actualizar_vehiculoActivo(db, id_usuario, id_vehiculo);

    char logmsg[128];
    snprintf(logmsg, sizeof(logmsg),
             "TRAYECTO iniciado usuario %d vehiculo %d trayecto %d",
             id_usuario, id_vehiculo, id_tray);
    log_escribir(cfg, logmsg);

    char resp[64];
    snprintf(resp, sizeof(resp), "%d", id_tray);
    net_ok(fd, resp);
}


static void cmd_fin_trayecto(int fd, sqlite3 *db, const Config *cfg,
                              int id_usuario, int id_trayecto,
                              float distancia, int id_est_destino) {

    /* Validar que la estación destino existe */
    if (id_est_destino <= 0) {
        net_error(fd, "Debes indicar una estacion de destino valida");
        return;
    }

    sqlite3_stmt *check;
    sqlite3_prepare_v2(db,
        "SELECT id_estacion FROM Estacion WHERE id_estacion = ?;",
        -1, &check, NULL);
    sqlite3_bind_int(check, 1, id_est_destino);
    int existe = (sqlite3_step(check) == SQLITE_ROW);
    sqlite3_finalize(check);

    if (!existe) {
        net_error(fd, "Estacion de destino no encontrada");
        return;
    }
    if (estacion_llena(db, id_est_destino))
    {
    char msg[128];

    snprintf(
        msg,
        sizeof(msg),
        "La estacion %d esta completa. No puedes dejar mas vehiculos.",
        id_est_destino);

    net_error(fd, msg);
    return;
    }

    char ahora[32];
    fecha_ahora(ahora, sizeof(ahora));

    sqlite3_stmt *s;
    sqlite3_prepare_v2(db,
        "UPDATE Trayecto "
        "SET fin=?, distancia=?, estacion_destino=? "
        "WHERE id_trayecto=? AND usuario_id=?;",
        -1, &s, NULL);
    sqlite3_bind_text(s, 1, ahora, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(s, 2, distancia);
    sqlite3_bind_int(s, 3, id_est_destino);
    sqlite3_bind_int(s, 4, id_trayecto);
    sqlite3_bind_int(s, 5, id_usuario);
    sqlite3_step(s);
    int cambios = sqlite3_changes(db);
    sqlite3_finalize(s);

    if (cambios == 0) {
        net_error(fd, "Trayecto no encontrado o no te pertenece");
        return;
    }

    Usuario u;
    buscar_usuario_por_id(db, id_usuario, &u);
    int id_veh = u.vehiculo_activo;

    actualizar_ubicacion_vehiculo(db, id_veh, id_est_destino);
    actualizar_estado(db, id_veh, "disponible");
    actualizar_vehiculoActivo(db, id_usuario, 0);

    char nombre_est[100] = "destino";
    sqlite3_stmt *nom;
    sqlite3_prepare_v2(db,
        "SELECT abreviacion FROM Estacion WHERE id_estacion = ?;",
        -1, &nom, NULL);
    sqlite3_bind_int(nom, 1, id_est_destino);
    if (sqlite3_step(nom) == SQLITE_ROW) {
        const char *n = (const char *)sqlite3_column_text(nom, 0);
        if (n) strncpy(nombre_est, n, sizeof(nombre_est) - 1);
    }
    sqlite3_finalize(nom);

    char logmsg[256];
    snprintf(logmsg, sizeof(logmsg),
             "TRAYECTO finalizado usuario %d trayecto %d dist %.1fkm destino estacion %d (%s)",
             id_usuario, id_trayecto, distancia, id_est_destino, nombre_est);
    log_escribir(cfg, logmsg);

    char resp[128];
    snprintf(resp, sizeof(resp),
             "Trayecto finalizado. Distancia: %.1f km. Vehiculo dejado en: %s",
             distancia, nombre_est);
    net_ok(fd, resp);
}


static void cmd_reportar_averia(int fd, sqlite3 *db, const Config *cfg,
                                 int id_usuario, const char *params) {
    int id_veh = 0;
    char tipo[32] = {0}, desc[256] = {0};
    sscanf(params, "%d %31s %255[^\n]", &id_veh, tipo, desc);

    if (id_veh <= 0 || tipo[0] == '\0') {
        net_error(fd, "Parametros incorrectos: REPORTAR_AV id_vehiculo tipo descripcion");
        return;
    }

    Vehiculo v;
    if (!buscar_vehiculo(db, id_veh, &v)) {
        net_error(fd, "Vehiculo no encontrado");
        return;
    }

    Averia a;
    memset(&a, 0, sizeof(a));
    a.id_vehiculo = id_veh;
    a.id_estacion = v.ubicacion_estacion;
    strncpy(a.tipo, tipo, sizeof(a.tipo) - 1);
    strncpy(a.descripcion, desc[0] ? desc : "Sin descripcion", sizeof(a.descripcion) - 1);
    strncpy(a.estado, "pendiente", sizeof(a.estado) - 1);
    fecha_ahora(a.fecha, sizeof(a.fecha));

    poner_averia(db, &a);
    actualizar_estado(db, id_veh, "averiado");

    char logmsg[256];
    snprintf(logmsg, sizeof(logmsg),
             "AVERIA reportada por usuario %d: vehiculo %d tipo %s",
             id_usuario, id_veh, tipo);
    log_escribir(cfg, logmsg);

    net_ok(fd, "Averia registrada. El vehiculo queda bloqueado hasta su reparacion.");
}


static void cmd_historial(int fd, sqlite3 *db, int id_usuario) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db,
        "SELECT t.id_trayecto, t.vehiculo_id, "
        "       eo.abreviacion, ed.abreviacion, "
        "       t.inicio, t.fin, t.distancia "
        "FROM Trayecto t "
        "LEFT JOIN Estacion eo ON t.estacion_origen  = eo.id_estacion "
        "LEFT JOIN Estacion ed ON t.estacion_destino = ed.id_estacion "
        "WHERE t.usuario_id = ? ORDER BY t.id_trayecto DESC LIMIT 20;",
        -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id_usuario);

    net_enviar(fd, RESP_OK);

    char linea[512];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int         id   = sqlite3_column_int(stmt, 0);
        int         veh  = sqlite3_column_int(stmt, 1);
        const char *orig = (const char *)sqlite3_column_text(stmt, 2);
        const char *dest = (const char *)sqlite3_column_text(stmt, 3);
        const char *ini  = (const char *)sqlite3_column_text(stmt, 4);
        const char *fin  = (const char *)sqlite3_column_text(stmt, 5);
        double      dis  = sqlite3_column_double(stmt, 6);
        snprintf(linea, sizeof(linea), "%d|%d|%s|%s|%s|%s|%.1f",
                 id, veh,
                 orig ? orig : "-",
                 dest ? dest : "en curso",
                 ini  ? ini  : "-",
                 fin  ? fin  : "en curso",
                 dis);
        net_enviar(fd, linea);
    }

    net_enviar(fd, RESP_FIN);
    sqlite3_finalize(stmt);
}


typedef struct {
    int              fd;
    sqlite3         *db;
    const Config    *cfg;
    pthread_mutex_t *mutex_db;
} HiloClienteArgs;

#ifdef _WIN32
static DWORD WINAPI hilo_cliente(void *arg) {
#else
static void *hilo_cliente(void *arg) {
#endif
    HiloClienteArgs *a  = (HiloClienteArgs *)arg;
    int              fd = a->fd;
    sqlite3         *db = a->db;
    const Config   *cfg = a->cfg;
    pthread_mutex_t *mx = a->mutex_db;
    free(a);

    int id_usuario = -1;

    printf("[SERVIDOR] Cliente conectado (fd=%d)\n", fd);
    net_enviar(fd, "BIENVENIDO Euskokar v1.0");

    char buffer[TAM_BUF];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;

        buffer[strcspn(buffer, "\r\n")] = '\0';
        if (strlen(buffer) == 0) continue;

        printf("[SERVIDOR] fd=%d recibio: '%s'\n", fd, buffer);

        char cmd[64]         = {0};
        char params[TAM_BUF] = {0};
        sscanf(buffer, "%63s %4031[^\n]", cmd, params);

        pthread_mutex_lock(mx);

        if (strcmp(cmd, CMD_LOGIN) == 0) {
            if (cmd_login(fd, db, params, &id_usuario)) {
                char logmsg[128];
                snprintf(logmsg, sizeof(logmsg),
                         "LOGIN cliente usuario_id %d", id_usuario);
                log_escribir(cfg, logmsg);
            }

        } else if (id_usuario < 0) {
            net_error(fd, "Primero debes hacer LOGIN");

        } else if (strcmp(cmd, CMD_LISTAR_EST) == 0) {
            cmd_listar_estaciones(fd, db);

        } else if (strcmp(cmd, CMD_VEH_ESTACION) == 0) {
            int id_est = atoi(params);
            cmd_vehiculos_estacion(fd, db, id_est);

        } else if (strcmp(cmd, CMD_LISTAR_VEH) == 0) {
            cmd_listar_vehiculos(fd, db);

        } else if (strcmp(cmd, CMD_RESERVAR) == 0) {
            int id_veh = atoi(params);
            cmd_reservar(fd, db, cfg, id_usuario, id_veh);

        } else if (strcmp(cmd, CMD_USAR_VEH) == 0) {
            int id_veh = atoi(params);
            cmd_usar_vehiculo(fd, db, cfg, id_usuario, id_veh);

        } else if (strcmp(cmd, CMD_FIN_TRAYECTO) == 0) {
            /* Nuevo formato: id_trayecto distancia id_estacion_destino */
            int   id_tray    = 0;
            float dist       = 0.0f;
            int   id_est_dst = 0;
            sscanf(params, "%d %f %d", &id_tray, &dist, &id_est_dst);
            cmd_fin_trayecto(fd, db, cfg, id_usuario, id_tray, dist, id_est_dst);

        } else if (strcmp(cmd, CMD_REPORTAR_AV) == 0) {
            cmd_reportar_averia(fd, db, cfg, id_usuario, params);

        } else if (strcmp(cmd, CMD_HISTORIAL) == 0) {
            cmd_historial(fd, db, id_usuario);

        } else if (strcmp(cmd, CMD_SALIR) == 0) {
            net_ok(fd, "Hasta luego");
            if (id_usuario >= 0) {
                char logmsg[128];
                snprintf(logmsg, sizeof(logmsg),
                         "SALIR cliente usuario_id %d", id_usuario);
                log_escribir(cfg, logmsg);
            }
            pthread_mutex_unlock(mx);
            break;

        } else {
            net_error(fd, "Comando desconocido");
        }

        pthread_mutex_unlock(mx);
    }

    printf("[SERVIDOR] Cliente desconectado (fd=%d)\n", fd);
    close(fd);
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}


#ifdef _WIN32
static DWORD WINAPI hilo_servidor(void *arg) {
#else
static void *hilo_servidor(void *arg) {
#endif
    ServidorArgs *sa = (ServidorArgs *)arg;

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    int servidor_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor_fd < 0) {
        perror("[SERVIDOR] Error al crear el socket");
#ifdef _WIN32
        return 1;
#else
        return NULL;
#endif
    }

    int opt = 1;
    setsockopt(servidor_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    struct sockaddr_in dir;
    memset(&dir, 0, sizeof(dir));
    dir.sin_family      = AF_INET;
    dir.sin_addr.s_addr = INADDR_ANY;
    dir.sin_port        = htons(sa->puerto);

    if (bind(servidor_fd, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
        perror("[SERVIDOR] Error en bind");
        close(servidor_fd);
#ifdef _WIN32
        return 1;
#else
        return NULL;
#endif
    }

    if (listen(servidor_fd, 10) < 0) {
        perror("[SERVIDOR] Error en listen");
        close(servidor_fd);
#ifdef _WIN32
        return 1;
#else
        return NULL;
#endif
    }

    printf("[SERVIDOR] Escuchando en puerto %d...\n", sa->puerto);

    while (1) {
        struct sockaddr_in dir_cliente;
        socklen_t tam = sizeof(dir_cliente);

        int cliente_fd = accept(servidor_fd, (struct sockaddr *)&dir_cliente, &tam);
        if (cliente_fd < 0) {
            perror("[SERVIDOR] Error en accept");
            continue;
        }

        printf("[SERVIDOR] Nueva conexion\n");

        HiloClienteArgs *ha = malloc(sizeof(HiloClienteArgs));
        ha->fd       = cliente_fd;
        ha->db       = sa->db;
        ha->cfg      = sa->cfg;
        ha->mutex_db = sa->mutex_db;

#ifdef _WIN32
        CloseHandle(CreateThread(NULL, 0, hilo_cliente, ha, 0, NULL));
#else
        pthread_t hilo;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&hilo, &attr, hilo_cliente, ha);
        pthread_attr_destroy(&attr);
#endif
    }

    close(servidor_fd);
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int servidor_iniciar(ServidorArgs *args, pthread_t *hilo) {
#ifdef _WIN32
    *hilo = CreateThread(NULL, 0, hilo_servidor, args, 0, NULL);
    return (*hilo != NULL) ? 1 : 0;
#else
    if (pthread_create(hilo, NULL, hilo_servidor, args) != 0) {
        perror("Error al crear el hilo del servidor");
        return 0;
    }
    return 1;
#endif
}