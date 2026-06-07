/*
 * ascii_mapa.cpp
 * Implementacion de los mapas ASCII segun el diseño de FASE_0.pdf.
 */

#include "ascii_mapa.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>

/* ------------------------------------------------------------------ */
/* Utilidades internas                                                  */
/* ------------------------------------------------------------------ */

/* Busca una estacion por abreviatura en el vector */
static const DatoEstacion *buscarEstacion(
        const std::vector<DatoEstacion> &ests,
        const std::string &abrev) {
    for (const auto &e : ests) {
        if (e.abrev == abrev) return &e;
    }
    return nullptr;
}

/*
 * simboloEstacion(e)
 * Devuelve el simbolo ASCII segun el estado de la estacion:
 *   ● si tiene vehiculos disponibles
 *   X si no tiene ninguno o todos averiados
 */
static std::string simboloEstacion(const DatoEstacion *e) {
    if (!e) return "?";
    return (e->disponibles > 0) ? "●" : "X";
}

/* Formatea una celda del mapa grande: "(AB)●" */
static std::string celda(const std::vector<DatoEstacion> &ests,
                          const std::string &abrev) {
    const DatoEstacion *e = buscarEstacion(ests, abrev);
    std::string sim = simboloEstacion(e);
    return "(" + abrev + ")" + sim;
}

/* ------------------------------------------------------------------ */
/* Mapa grande de Gipuzkoa                                             */
/* ------------------------------------------------------------------ */
/*
 * El mapa sigue la disposicion geografica aproximada definida en FASE_0.pdf.
 * Cada estacion se identifica por su abreviatura y un simbolo de estado.
 * Añade tu mapa o modifica las abreviaturas segun las que tengais en el CSV.
 */
void dibujar_mapa_grande(const std::vector<DatoEstacion> &estaciones,
                          int /*id_vehiculo_usuario*/) {

    std::cout << "\n";
    std::cout << " +=========================================================+\n";
    std::cout << " |           MAPA EUSKOKAR - GIPUZKOA                      |\n";
    std::cout << " |   ● disponible   X sin vehiculos / averiada             |\n";
    std::cout << " +=========================================================+\n";
    std::cout << " |                                                         |\n";

    /* Fila 1: costa norte (Zarautz, Donostia, Errenteria, Hondarribia) */
    std::cout << " |  "
              << std::left << std::setw(8) << celda(estaciones, "Z")
              << std::setw(8) << celda(estaciones, "DC")
              << std::setw(8) << celda(estaciones, "DG")
              << std::setw(8) << celda(estaciones, "E")
              << std::setw(8) << celda(estaciones, "H")
              << "       |\n";

    /* Fila 2: zona costera interior */
    std::cout << " |  "
              << std::setw(8) << celda(estaciones, "DA")
              << std::setw(8) << celda(estaciones, "DA2")
              << std::setw(8) << celda(estaciones, "DL")
              << std::setw(8) << celda(estaciones, "P")
              << "               |\n";

    std::cout << " |                                                         |\n";

    /* Fila 3: zona interior norte */
    std::cout << " |     "
              << std::setw(8) << celda(estaciones, "U")
              << std::setw(8) << celda(estaciones, "T")
              << std::setw(8) << celda(estaciones, "EB")
              << "                    |\n";

    /* Fila 4: zona interior */
    std::cout << " |     "
              << std::setw(8) << celda(estaciones, "A")
              << std::setw(8) << celda(estaciones, "L")
              << std::setw(8) << celda(estaciones, "HE")
              << std::setw(8) << celda(estaciones, "O")
              << "          |\n";

    /* Fila 5: sur */
    std::cout << " |        "
              << std::setw(8) << celda(estaciones, "B")
              << std::setw(8) << celda(estaciones, "S")
              << std::setw(8) << celda(estaciones, "M")
              << "               |\n";

    std::cout << " |                                                         |\n";
    std::cout << " +=========================================================+\n\n";

    /* Leyenda con conteos */
    int total_disp = 0, total_est = (int)estaciones.size();
    for (const auto &e : estaciones) total_disp += e.disponibles;
    std::cout << " Estaciones: " << total_est
              << "   Vehiculos disponibles: " << total_disp << "\n\n";
}

/* ------------------------------------------------------------------ */
/* Minimapa de una estacion                                             */
/* ------------------------------------------------------------------ */

void dibujar_minimapa(const std::string &nombre_estacion,
                      const std::vector<DatoVehiculo> &vehiculos,
                      int id_vehiculo_usuario) {

    /* Calcular el ancho del marco segun el numero de vehiculos */
    int n = (int)vehiculos.size();
    int ancho = std::max(30, n * 7 + 4);

    /* Linea superior */
    std::string borde(ancho, '=');
    std::cout << "\n ╔" << borde << "╗\n";

    /* Nombre de la estacion (centrado) */
    std::string titulo = " Estacion: " + nombre_estacion + " ";
    int pad = (ancho - (int)titulo.size()) / 2;
    std::cout << " ║" << std::string(pad, ' ') << titulo
              << std::string(ancho - pad - (int)titulo.size(), ' ') << "║\n";

    std::cout << " ║" << std::string(ancho, '-') << "║\n";

    /* Fila de vehiculos */
    std::string fila_vehiculos = " ";
    int disp = 0, av = 0, en_uso = 0;

    for (const auto &v : vehiculos) {
        std::string celda_v;
        if (v.id == id_vehiculo_usuario) {
            celda_v = "[*V" + std::to_string(v.id) + "*]";
        } else if (v.estado == "disponible") {
            celda_v = "[ V" + std::to_string(v.id) + " ]";
            disp++;
        } else if (v.estado == "averiado") {
            celda_v = "[ X" + std::to_string(v.id) + " ]";
            av++;
        } else { /* en_uso */
            celda_v = "[ ~" + std::to_string(v.id) + " ]";
            en_uso++;
        }
        fila_vehiculos += celda_v;
    }

    if (vehiculos.empty()) {
        fila_vehiculos = " (Sin vehiculos asignados a esta estacion)";
    }

    /* Centrar la fila */
    int fpad = (ancho - (int)fila_vehiculos.size()) / 2;
    if (fpad < 0) fpad = 0;
    std::cout << " ║" << std::string(fpad, ' ') << fila_vehiculos
              << std::string(std::max(0, ancho - fpad - (int)fila_vehiculos.size()), ' ')
              << "║\n";

    /* Leyenda */
    std::cout << " ║" << std::string(ancho, '-') << "║\n";
    std::string leyenda = " [V]=libre [~]=en uso [X]=averiado [*V*]=tuyo ";
    std::cout << " ║" << leyenda
              << std::string(std::max(0, ancho - (int)leyenda.size()), ' ')
              << "║\n";

    /* Estadisticas */
    std::string stats = " Disp:" + std::to_string(disp) +
                        " EnUso:" + std::to_string(en_uso) +
                        " Averiados:" + std::to_string(av) + " ";
    std::cout << " ║" << stats
              << std::string(std::max(0, ancho - (int)stats.size()), ' ')
              << "║\n";

    /* Linea inferior */
    std::cout << " ╚" << borde << "╝\n\n";
}
