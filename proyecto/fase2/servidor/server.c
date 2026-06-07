/*
 * server.c
 * Servidor de sockets TCP para Euskokar.
 *
 * ¿Que es un socket?
 * ------------------
 * Un socket es como un "enchufe de red". Permite que dos programas (en la
 * misma maquina o en maquinas distintas) se comuniquen enviando y recibiendo
 * cadenas de texto.
 *
 * Lado servidor (este fichero):
 *   1. socket()  → crea el enchufe
 *   2. bind()    → le asigna un puerto (numero de puerta, p.ej. 8080)
 *   3. listen()  → empieza a esperar llamadas entrantes
 *   4. accept()  → acepta una conexion de un cliente → devuelve nuevo fd
 *   5. recv()/send() → recibir/enviar datos por ese fd
 *   6. close()   → cierra la conexion
 *
 * Cada cliente que se conecta se gestiona en un hilo (pthread) separado,
 * para que varios usuarios puedan conectarse a la vez.
 * El acceso a la base de datos se protege con un mutex.
 */

#include "server.h"
#include "db.h"
#include "logic.h"
#include "protocolo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

/* Headers de red (Linux/Unix) */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* ------------------------------------------------------------------ */
/* Utilidades de red                                                    */
/* ------------------------------------------------------------------ */

/* Envia una cadena al cliente asegurandose de que se manda completa */
static void net_enviar(int fd, const char *msg) {
    char buf[TAM_BUF];
    int n = snprintf(buf, sizeof(buf), "%s\n", msg);
    send(fd, buf, n, 0);
}

/* Envia "OK dato" */
static void net_ok(int fd, const char *dato) {
    char buf[TAM_BUF];
    if (dato && dato[0] != '\0')
        snprintf(buf, sizeof(buf), "OK %s", dato);
    else
        snprintf(buf, sizeof(buf), "OK");
    net_enviar(fd, buf);
}

/* Envia "ERROR mensaje" */
static void net_error(int fd, const char *msg) {
    char buf[TAM_BUF];
    snprintf(buf, sizeof(buf), "ERROR %s", msg);
    net_enviar(fd, buf);
}

/* ------------------------------------------------------------------ */
/* Funciones que consultan la BD y envian los datos al cliente         */
/* ------------------------------------------------------------------ */

/*
 * Envia la lista de estaciones con disponibilidad en tiempo real.
 * Formato de cada linea: id|abrev|nombre|plazas|disponibles
 */
static void cmd_listar_estaciones(int fd, sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT e.id_estacion, e.abreviacion, e.nombre, e.plazas, "
        "       COUNT(CASE WHEN v.estado='disponible' THEN 1 END) AS disponibles "
        "FROM Estacion e "
        "LEFT JOIN Vehiculo v ON e.id_estacion = v.ubicacion_estacion "
        "GROUP BY e.id_estacion "
        "ORDER BY e.id_estacion;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        net_error(fd, "Error interno al listar estaciones");
        return;
    }

    net_enviar(fd, RESP_OK);   /* Cabecera: OK (viene la lista) */

    char linea[256];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int    id    = sqlite3_column_int(stmt, 0);
        const char *ab   = (const char *)sqlite3_column_text(stmt, 1);
        const char *nom  = (const char *)sqlite3_column_text(stmt, 2);
        int    plaz = sqlite3_column_int(stmt, 3);
        int    disp = sqlite3_column_int(stmt, 4);
        snprintf(linea, sizeof(linea), "%d|%s|%s|%d|%d",
                 id,
                 ab  ? ab  : "?",
                 nom ? nom : "?",
                 plaz, disp);
        net_enviar(fd, linea);
    }

    net_enviar(fd, RESP_FIN);  /* Fin de lista */
    sqlite3_finalize(stmt);
}

/*
 * Envia los vehiculos de una estacion concreta.
 * Formato: id_vehiculo|estado|bateria
 */
static void cmd_vehiculos_estacion(int fd, sqlite3 *db, int id_estacion) {
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT id_vehiculo, estado, bateria "
        "FROM Vehiculo WHERE ubicacion_estacion = ? "
        "ORDER BY id_vehiculo;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id_estacion);

    net_enviar(fd, RESP_OK);

    char linea[128];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int        id  = sqlite3_column_int(stmt, 0);
        const char *est = (const char *)sqlite3_column_text(stmt, 1);
        double     bat = sqlite3_column_double(stmt, 2);
        snprintf(linea, sizeof(linea), "%d|%s|%.1f",
                 id, est ? est : "?", bat);
        net_enviar(fd, linea);
    }

    net_enviar(fd, RESP_FIN);
    sqlite3_finalize(stmt);
}

/*
 * Envia todos los vehiculos del sistema.
 * Formato: id|estado|bateria|abrev_estacion
 */
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
        int        id  = sqlite3_column_int(stmt, 0);
        const char *est = (const char *)sqlite3_column_text(stmt, 1);
        double     bat = sqlite3_column_double(stmt, 2);
        const char *ab  = (const char *)sqlite3_column_text(stmt, 3);
        snprintf(linea, sizeof(linea), "%d|%s|%.1f|%s",
                 id,
                 est ? est : "?",
                 bat,
                 ab  ? ab  : "-");
        net_enviar(fd, linea);
    }

    net_enviar(fd, RESP_FIN);
    sqlite3_finalize(stmt);
}

/*
 * Procesa el LOGIN del usuario.
 * Comprueba nombre y contraseña en la BD.
 */
static int cmd_login(int fd, sqlite3 *db, const char *params,
                     int *id_usuario_out) {
    char nombre[100] = {0}, clave[100] = {0};
    sscanf(params, "%99s %99s", nombre, clave);

    Usuario u;
    if (!buscar_usuario_por_nombre(db, nombre, &u)) {
        net_error(fd, "Usuario no encontrado");
        return 0;
    }

    if (strcmp(u.contrasenya, clave) != 0) {
        net_error(fd, "Contraseña incorrecta");
        return 0;
    }

    /* Login correcto */
    *id_usuario_out = u.id_usuario;
    char resp[256];
    snprintf(resp, sizeof(resp), "%d %s", u.id_usuario, u.nombre);
    net_ok(fd, resp);
    return 1;
}

/*
 * Reserva un vehiculo para el usuario.
 * Comprueba que el vehiculo esta disponible y que el usuario
 * no tiene ya otro vehiculo activo.
 */
static void cmd_reservar(int fd, sqlite3 *db, const Config *cfg,
                          int id_usuario, int id_vehiculo) {
    /* ¿Ya tiene vehiculo activo? */
    Usuario u;
    if (!buscar_usuario_por_id(db, id_usuario, &u)) {
        net_error(fd, "Usuario no encontrado");
        return;
    }
    if (u.vehiculo_activo != 0) {
        net_error(fd, "Ya tienes un vehiculo activo. Finaliza el trayecto primero.");
        return;
    }

    /* ¿El vehiculo esta disponible? */
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

    /* Crear reserva */
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

    /* Log */
    char logmsg[128];
    snprintf(logmsg, sizeof(logmsg),
             "RESERVA usuario %d vehiculo %d", id_usuario, id_vehiculo);
    log_escribir(cfg, logmsg);

    char resp[64];
    snprintf(resp, sizeof(resp), "Reserva creada para vehiculo %d", id_vehiculo);
    net_ok(fd, resp);
}

/*
 * Inicia el uso de un vehiculo (crea un trayecto).
 * Marca el vehiculo como "en_uso" y lo asocia al usuario.
 */
static void cmd_usar_vehiculo(int fd, sqlite3 *db, const Config *cfg,
                               int id_usuario, int id_vehiculo) {
    /* Comprobaciones */
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

    /* Crear trayecto con fin vacio (se rellena al finalizar) */
    char ahora[32];
    fecha_ahora(ahora, sizeof(ahora));
    Trayecto t;
    memset(&t, 0, sizeof(t));
    t.usuario_id  = id_usuario;
    t.vehiculo_id = id_vehiculo;
    strncpy(t.inicio, ahora, sizeof(t.inicio) - 1);
    t.distancia = 0.0f;

    insertar_trayecto(db, &t);

    /* Obtener el id del trayecto recien insertado */
    sqlite3_stmt *s;
    int id_tray = 0;
    sqlite3_prepare_v2(db, "SELECT MAX(id_trayecto) FROM Trayecto;", -1, &s, NULL);
    if (sqlite3_step(s) == SQLITE_ROW) id_tray = sqlite3_column_int(s, 0);
    sqlite3_finalize(s);

    /* Actualizar estado del vehiculo y vehiculo activo del usuario */
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

/*
 * Finaliza un trayecto activo.
 * Libera el vehiculo y actualiza distancia.
 */
static void cmd_fin_trayecto(int fd, sqlite3 *db, const Config *cfg,
                              int id_usuario, int id_trayecto, float distancia) {
    /* Marcar fin del trayecto en la BD */
    char ahora[32];
    fecha_ahora(ahora, sizeof(ahora));

    sqlite3_stmt *s;
    sqlite3_prepare_v2(db,
        "UPDATE Trayecto SET fin=?, distancia=? WHERE id_trayecto=? AND usuario_id=?;",
        -1, &s, NULL);
    sqlite3_bind_text(s, 1, ahora, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(s, 2, distancia);
    sqlite3_bind_int(s, 3, id_trayecto);
    sqlite3_bind_int(s, 4, id_usuario);
    sqlite3_step(s);
    int cambios = sqlite3_changes(db);
    sqlite3_finalize(s);

    if (cambios == 0) {
        net_error(fd, "Trayecto no encontrado o no te pertenece");
        return;
    }

    /* Liberar el vehiculo */
    Usuario u;
    buscar_usuario_por_id(db, id_usuario, &u);
    int id_veh = u.vehiculo_activo;

    actualizar_estado(db, id_veh, "disponible");
    actualizar_vehiculoActivo(db, id_usuario, 0);

    char logmsg[128];
    snprintf(logmsg, sizeof(logmsg),
             "TRAYECTO finalizado usuario %d trayecto %d dist %.1fkm",
             id_usuario, id_trayecto, distancia);
    log_escribir(cfg, logmsg);

    char resp[64];
    snprintf(resp, sizeof(resp), "Trayecto finalizado. Distancia: %.1f km", distancia);
    net_ok(fd, resp);
}

/*
 * Registra una averia de un vehiculo.
 * Formato params: id_vehiculo tipo descripcion
 */
static void cmd_reportar_averia(int fd, sqlite3 *db, const Config *cfg,
                                 int id_usuario, const char *params) {
    int id_veh = 0;
    char tipo[32] = {0}, desc[256] = {0};

    /* Parsear: primer token = id, segundo = tipo, resto = descripcion */
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
    a.id_estacion = 0;
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

/*
 * Envia el historial de trayectos del usuario.
 * Formato: id_trayecto|id_vehiculo|inicio|fin|distancia
 */
static void cmd_historial(int fd, sqlite3 *db, int id_usuario) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db,
        "SELECT id_trayecto, vehiculo_id, inicio, fin, distancia "
        "FROM Trayecto WHERE usuario_id = ? ORDER BY id_trayecto DESC LIMIT 20;",
        -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id_usuario);

    net_enviar(fd, RESP_OK);

    char linea[256];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int        id  = sqlite3_column_int(stmt, 0);
        int        veh = sqlite3_column_int(stmt, 1);
        const char *ini = (const char *)sqlite3_column_text(stmt, 2);
        const char *fin = (const char *)sqlite3_column_text(stmt, 3);
        double     dis = sqlite3_column_double(stmt, 4);
        snprintf(linea, sizeof(linea), "%d|%d|%s|%s|%.1f",
                 id, veh,
                 ini ? ini : "-",
                 fin ? fin : "en curso",
                 dis);
        net_enviar(fd, linea);
    }

    net_enviar(fd, RESP_FIN);
    sqlite3_finalize(stmt);
}

/* ------------------------------------------------------------------ */
/* Hilo que gestiona un cliente conectado                              */
/* ------------------------------------------------------------------ */

typedef struct {
    int              fd;
    sqlite3         *db;
    const Config    *cfg;
    pthread_mutex_t *mutex_db;
} HiloClienteArgs;

static void *hilo_cliente(void *arg) {
    HiloClienteArgs *a  = (HiloClienteArgs *)arg;
    int              fd = a->fd;
    sqlite3         *db = a->db;
    const Config   *cfg = a->cfg;
    pthread_mutex_t *mx = a->mutex_db;
    free(a);

    int id_usuario   = -1;   /* -1 = no ha hecho login */

    printf("[SERVIDOR] Cliente conectado (fd=%d)\n", fd);
    net_enviar(fd, "BIENVENIDO Euskokar v1.0");

    char buffer[TAM_BUF];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;  /* Cliente desconectado */

        /* Quitar \r\n al final */
        buffer[strcspn(buffer, "\r\n")] = '\0';
        if (strlen(buffer) == 0) continue;

        printf("[SERVIDOR] fd=%d recibio: '%s'\n", fd, buffer);

        /* Separar comando de parametros */
        char cmd[64]        = {0};
        char params[TAM_BUF] = {0};
        sscanf(buffer, "%63s %4031[^\n]", cmd, params);

        /* ---- Bloquear BD antes de operar ---- */
        pthread_mutex_lock(mx);

        /* LOGIN - no requiere sesion previa */
        if (strcmp(cmd, CMD_LOGIN) == 0) {
            cmd_login(fd, db, params, &id_usuario);

        /* Resto de comandos requieren login previo */
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
            /* Guardar id del trayecto recien creado */
            /* (el id se envia en la respuesta "OK id_trayecto") */

        } else if (strcmp(cmd, CMD_FIN_TRAYECTO) == 0) {
            int   id_tray = 0;
            float dist    = 0.0f;
            sscanf(params, "%d %f", &id_tray, &dist);
            cmd_fin_trayecto(fd, db, cfg, id_usuario, id_tray, dist);

        } else if (strcmp(cmd, CMD_REPORTAR_AV) == 0) {
            cmd_reportar_averia(fd, db, cfg, id_usuario, params);

        } else if (strcmp(cmd, CMD_HISTORIAL) == 0) {
            cmd_historial(fd, db, id_usuario);

        } else if (strcmp(cmd, CMD_SALIR) == 0) {
            net_ok(fd, "Hasta luego");
            pthread_mutex_unlock(mx);
            break;

        } else {
            net_error(fd, "Comando desconocido");
        }

        pthread_mutex_unlock(mx);
    }

    printf("[SERVIDOR] Cliente desconectado (fd=%d)\n", fd);
    close(fd);
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Hilo principal del servidor: acepta conexiones                     */
/* ------------------------------------------------------------------ */

static void *hilo_servidor(void *arg) {
    ServidorArgs *sa = (ServidorArgs *)arg;

    /* 1. Crear el socket del servidor */
    int servidor_fd = socket(AF_INET,     /* IPv4              */
                             SOCK_STREAM, /* TCP (fiable)      */
                             0);          /* protocolo por defecto */
    if (servidor_fd < 0) {
        perror("[SERVIDOR] Error al crear el socket");
        return NULL;
    }

    /* Permitir reutilizar el puerto rapidamente tras reiniciar */
    int opt = 1;
    setsockopt(servidor_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* 2. Asociar el socket a un puerto (bind) */
    struct sockaddr_in dir;
    memset(&dir, 0, sizeof(dir));
    dir.sin_family      = AF_INET;
    dir.sin_addr.s_addr = INADDR_ANY;   /* Acepta conexiones de cualquier IP */
    dir.sin_port        = htons(sa->puerto);

    if (bind(servidor_fd, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
        perror("[SERVIDOR] Error en bind");
        close(servidor_fd);
        return NULL;
    }

    /* 3. Empezar a escuchar (hasta 10 conexiones en cola) */
    if (listen(servidor_fd, 10) < 0) {
        perror("[SERVIDOR] Error en listen");
        close(servidor_fd);
        return NULL;
    }

    printf("[SERVIDOR] Escuchando en puerto %d...\n", sa->puerto);

    /* 4. Bucle principal: aceptar y gestionar clientes */
    while (1) {
        struct sockaddr_in dir_cliente;
        socklen_t tam = sizeof(dir_cliente);

        /* accept() bloquea hasta que llega un cliente */
        int cliente_fd = accept(servidor_fd,
                                (struct sockaddr *)&dir_cliente,
                                &tam);
        if (cliente_fd < 0) {
            perror("[SERVIDOR] Error en accept");
            continue;
        }

        printf("[SERVIDOR] Nueva conexion desde %s\n",
               inet_ntoa(dir_cliente.sin_addr));

        /* Preparar argumentos para el hilo del cliente */
        HiloClienteArgs *ha = malloc(sizeof(HiloClienteArgs));
        ha->fd       = cliente_fd;
        ha->db       = sa->db;
        ha->cfg      = sa->cfg;
        ha->mutex_db = sa->mutex_db;

        /* Crear un hilo nuevo para gestionar este cliente */
        pthread_t hilo;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&hilo, &attr, hilo_cliente, ha);
        pthread_attr_destroy(&attr);
    }

    close(servidor_fd);
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Funcion publica: lanza el servidor en un hilo separado             */
/* ------------------------------------------------------------------ */

int servidor_iniciar(ServidorArgs *args, pthread_t *hilo) {
    if (pthread_create(hilo, NULL, hilo_servidor, args) != 0) {
        perror("Error al crear el hilo del servidor");
        return 0;
    }
    return 1;
}
