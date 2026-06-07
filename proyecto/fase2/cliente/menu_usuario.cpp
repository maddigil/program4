/*
 * menu_usuario.cpp
 * Implementacion del menu de usuario del cliente C++ de Euskokar.
 *
 * Cada opcion del menu:
 *   1. Construye el comando de protocolo (p.ej. "LISTAR_EST")
 *   2. Lo envia al servidor via cli.enviarComando()
 *   3. Lee la respuesta con cli.leerLinea() o cli.leerLista()
 *   4. Muestra el resultado al usuario (con ASCII art si procede)
 */

#include "menu_usuario.h"
#include "ascii_mapa.h"
#include "protocolo.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <iomanip>

/* ------------------------------------------------------------------ */
/* Utilidades de entrada                                                */
/* ------------------------------------------------------------------ */

static int leerEntero(const std::string &prompt, int minVal, int maxVal) {
    int val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val && val >= minVal && val <= maxVal) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return val;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << " Valor invalido. Introduce un numero entre "
                  << minVal << " y " << maxVal << ".\n";
    }
}

static std::string leerCadena(const std::string &prompt) {
    std::string s;
    std::cout << prompt;
    std::getline(std::cin, s);
    return s;
}

static float leerFlotante(const std::string &prompt) {
    float v;
    while (true) {
        std::cout << prompt;
        if (std::cin >> v && v >= 0.0f) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return v;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << " Valor invalido.\n";
    }
}

/* ------------------------------------------------------------------ */
/* Parseo de campos separados por '|'                                  */
/* ------------------------------------------------------------------ */

static std::vector<std::string> split(const std::string &s, char sep) {
    std::vector<std::string> partes;
    std::stringstream ss(s);
    std::string parte;
    while (std::getline(ss, parte, sep)) {
        partes.push_back(parte);
    }
    return partes;
}

/* ------------------------------------------------------------------ */
/* Opciones del menu                                                    */
/* ------------------------------------------------------------------ */

/*
 * Opcion 1: Mapa de Gipuzkoa
 * Pide LISTAR_EST y dibuja el mapa grande.
 */
static void op_mapa_gipuzkoa(Cliente &cli, int id_veh_activo) {
    cli.enviarComando(CMD_LISTAR_EST);
    auto lineas = cli.leerLista();

    std::vector<DatoEstacion> estaciones;
    for (const auto &l : lineas) {
        auto p = split(l, '|');
        if (p.size() < 5) continue;
        DatoEstacion e;
        e.id          = std::stoi(p[0]);
        e.abrev       = p[1];
        e.nombre      = p[2];
        e.plazas      = std::stoi(p[3]);
        e.disponibles = std::stoi(p[4]);
        estaciones.push_back(e);
    }

    dibujar_mapa_grande(estaciones, id_veh_activo);
}

/*
 * Opcion 2: Ver una estacion especifica (minimapa)
 */
static void op_ver_estacion(Cliente &cli, int id_veh_activo) {
    int id_est = leerEntero(" ID de la estacion: ", 1, 9999);

    std::string cmd = std::string(CMD_VEH_ESTACION) + " " + std::to_string(id_est);
    cli.enviarComando(cmd);
    auto lineas = cli.leerLista();

    /* Necesitamos tambien el nombre de la estacion */
    cli.enviarComando(CMD_LISTAR_EST);
    auto todas = cli.leerLista();
    std::string nombre_est = "Estacion " + std::to_string(id_est);
    for (const auto &l : todas) {
        auto p = split(l, '|');
        if (p.size() >= 3 && std::stoi(p[0]) == id_est) {
            nombre_est = p[2] + " (" + p[1] + ")";
            break;
        }
    }

    std::vector<DatoVehiculo> vehiculos;
    for (const auto &l : lineas) {
        auto p = split(l, '|');
        if (p.size() < 3) continue;
        DatoVehiculo v;
        v.id      = std::stoi(p[0]);
        v.estado  = p[1];
        v.bateria = std::stof(p[2]);
        vehiculos.push_back(v);
    }

    dibujar_minimapa(nombre_est, vehiculos, id_veh_activo);
}

/*
 * Opcion 3: Reservar un vehiculo
 */
static void op_reservar(Cliente &cli, int /*id_usuario*/) {
    /* Primero mostramos los disponibles */
    cli.enviarComando(CMD_LISTAR_VEH);
    auto lineas = cli.leerLista();

    std::cout << "\n  Vehiculos disponibles:\n";
    std::cout << "  " << std::string(45, '-') << "\n";
    std::cout << "  " << std::left
              << std::setw(6) << "ID"
              << std::setw(14) << "Estado"
              << std::setw(10) << "Bateria"
              << std::setw(8)  << "Estacion"
              << "\n";
    std::cout << "  " << std::string(45, '-') << "\n";

    bool hay_disp = false;
    for (const auto &l : lineas) {
        auto p = split(l, '|');
        if (p.size() < 4) continue;
        if (p[1] != "disponible") continue;
        hay_disp = true;
        std::cout << "  "
                  << std::setw(6) << p[0]
                  << std::setw(14) << p[1]
                  << std::setw(9)  << p[2] << "%"
                  << std::setw(8)  << p[3]
                  << "\n";
    }

    if (!hay_disp) {
        std::cout << "  No hay vehiculos disponibles en este momento.\n\n";
        return;
    }
    std::cout << "\n";

    int id_veh = leerEntero(" ID del vehiculo a reservar (0 para cancelar): ", 0, 99999);
    if (id_veh == 0) return;

    std::string cmd = std::string(CMD_RESERVAR) + " " + std::to_string(id_veh);
    cli.enviarComando(cmd);
    std::string resp = cli.leerLinea();

    if (Cliente::esOk(resp)) {
        std::cout << "\n  Reserva realizada: " << Cliente::valorOk(resp) << "\n\n";
    } else {
        std::cout << "\n  Error: " << Cliente::mensajeError(resp) << "\n\n";
    }
}

/*
 * Opcion 4: Desbloquear (usar) el vehiculo reservado
 */
static int op_usar_vehiculo(Cliente &cli) {
    int id_veh = leerEntero(" ID del vehiculo a desbloquear: ", 1, 99999);

    std::string cmd = std::string(CMD_USAR_VEH) + " " + std::to_string(id_veh);
    cli.enviarComando(cmd);
    std::string resp = cli.leerLinea();

    if (Cliente::esOk(resp)) {
        int id_tray = std::stoi(Cliente::valorOk(resp));
        std::cout << "\n  Vehiculo desbloqueado. Trayecto iniciado (ID: "
                  << id_tray << ").\n";
        std::cout << "  Recuerda finalizar el trayecto cuando aparques.\n\n";
        return id_tray;
    } else {
        std::cout << "\n  Error: " << Cliente::mensajeError(resp) << "\n\n";
        return 0;
    }
}

/*
 * Opcion 5: Finalizar trayecto
 */
static void op_finalizar_trayecto(Cliente &cli, int &id_tray_activo) {
    if (id_tray_activo == 0) {
        std::cout << "\n  No tienes ningun trayecto activo.\n\n";
        return;
    }

    std::cout << "\n  Trayecto activo: ID " << id_tray_activo << "\n";
    float dist = leerFlotante(" Distancia recorrida (km): ");

    std::string cmd = std::string(CMD_FIN_TRAYECTO) + " " +
                      std::to_string(id_tray_activo) + " " +
                      std::to_string(dist);
    cli.enviarComando(cmd);
    std::string resp = cli.leerLinea();

    if (Cliente::esOk(resp)) {
        std::cout << "\n  " << Cliente::valorOk(resp) << "\n";
        std::cout << "  El vehiculo ha sido liberado. ¡Gracias!\n\n";
        id_tray_activo = 0;
    } else {
        std::cout << "\n  Error: " << Cliente::mensajeError(resp) << "\n\n";
    }
}

/*
 * Opcion 6: Reportar averia
 */
static void op_reportar_averia(Cliente &cli) {
    std::cout << "\n  +----------------------------------+\n";
    std::cout << "  |      REPORTAR AVERIA             |\n";
    std::cout << "  +----------------------------------+\n";

    int id_veh = leerEntero(" ID del vehiculo: ", 1, 99999);

    std::cout << "  Tipo de averia:\n";
    std::cout << "    1. mecanica\n    2. bateria\n    3. carga\n    4. otro\n";
    int t = leerEntero(" Selecciona (1-4): ", 1, 4);
    const char *tipos[] = {"mecanica", "bateria", "carga", "otro"};

    std::string desc = leerCadena(" Descripcion breve: ");

    std::string cmd = std::string(CMD_REPORTAR_AV) + " " +
                      std::to_string(id_veh) + " " +
                      tipos[t-1] + " " +
                      (desc.empty() ? "sin descripcion" : desc);
    cli.enviarComando(cmd);
    std::string resp = cli.leerLinea();

    if (Cliente::esOk(resp)) {
        std::cout << "\n  " << Cliente::valorOk(resp) << "\n\n";
    } else {
        std::cout << "\n  Error: " << Cliente::mensajeError(resp) << "\n\n";
    }
}

/*
 * Opcion 7: Historial de trayectos
 */
static void op_historial(Cliente &cli) {
    cli.enviarComando(CMD_HISTORIAL);
    auto lineas = cli.leerLista();

    std::cout << "\n  Historial de trayectos:\n";
    std::cout << "  " << std::string(65, '-') << "\n";
    std::cout << "  " << std::left
              << std::setw(6) << "ID"
              << std::setw(8) << "Vehic."
              << std::setw(20) << "Inicio"
              << std::setw(20) << "Fin"
              << std::setw(10) << "Dist(km)"
              << "\n";
    std::cout << "  " << std::string(65, '-') << "\n";

    if (lineas.empty()) {
        std::cout << "  (Sin trayectos registrados)\n";
    }

    for (const auto &l : lineas) {
        auto p = split(l, '|');
        if (p.size() < 5) continue;
        std::cout << "  "
                  << std::setw(6) << p[0]
                  << std::setw(8) << p[1]
                  << std::setw(20) << p[2]
                  << std::setw(20) << p[3]
                  << std::setw(10) << p[4]
                  << "\n";
    }
    std::cout << "\n";
}

/* ------------------------------------------------------------------ */
/* Menu principal del usuario                                           */
/* ------------------------------------------------------------------ */

void menu_principal(Cliente &cli, int id_usuario, const std::string &nombre) {
    int  op             = -1;
    int  id_tray_activo = 0;   /* 0 = sin trayecto activo */
    int  id_veh_activo  = 0;   /* 0 = sin vehiculo activo */

    while (op != 0 && cli.conectado()) {
        std::cout << "\n +=================================+\n";
        std::cout << " |  EUSKOKAR - Menu usuario        |\n";
        std::cout << " |  Sesion: " << std::left << std::setw(22) << nombre << "|\n";
        if (id_tray_activo)
            std::cout << " |  * Trayecto activo: "
                      << std::setw(10) << id_tray_activo << "        |\n";
        std::cout << " +=================================+\n";
        std::cout << " 1. Ver mapa de Gipuzkoa\n";
        std::cout << " 2. Ver estacion concreta (minimapa)\n";
        std::cout << " 3. Reservar un vehiculo\n";
        std::cout << " 4. Desbloquear / iniciar trayecto\n";
        std::cout << " 5. Finalizar trayecto activo\n";
        std::cout << " 6. Reportar averia\n";
        std::cout << " 7. Ver mi historial de trayectos\n";
        std::cout << " 0. Salir\n";

        op = leerEntero(" Opcion: ", 0, 7);

        switch (op) {
            case 1: op_mapa_gipuzkoa(cli, id_veh_activo);               break;
            case 2: op_ver_estacion(cli, id_veh_activo);                 break;
            case 3: op_reservar(cli, id_usuario);                        break;
            case 4:
                id_tray_activo = op_usar_vehiculo(cli);
                if (id_tray_activo) id_veh_activo = 0; /* lo actualizamos */
                break;
            case 5:
                op_finalizar_trayecto(cli, id_tray_activo);
                if (!id_tray_activo) id_veh_activo = 0;
                break;
            case 6: op_reportar_averia(cli);                             break;
            case 7: op_historial(cli);                                   break;
            case 0: break;
        }
    }

    /* Notificar al servidor que el cliente cierra */
    cli.enviarComando(CMD_SALIR);
    cli.leerLinea();
}
