//en comentarios explicacion breve para entender cada metodo:
#include "ascii_mapa.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>


/* Utilidades internas del proyecto:                                         */

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
 *   * si tiene vehiculos disponibles
 *   X si no tiene ninguno o todos averiados
 */
static std::string simboloEstacion(const DatoEstacion *e) {
    if (!e) return "?";
    return (e->disponibles > 0) ? "*" : "X";
}

/* Formatea una celda del mapa grande: "(AB)*" */
static std::string celda(const std::vector<DatoEstacion> &ests,
                          const std::string &abrev) {
    const DatoEstacion *e = buscarEstacion(ests, abrev);
    std::string sim = simboloEstacion(e);
    return "(" + abrev + ")" + sim;
}

/* ------------------------------------------------------------------ */
/* Mapa grande de Gipuzkoa                                             */
/* ------------------------------------------------------------------ */
void dibujar_mapa_grande(const std::vector<DatoEstacion> &estaciones,
                          int /*id_vehiculo_usuario*/) {

    std::cout << "\n";
    std::cout << " +=========================================================+\n";
    std::cout << " |           MAPA EUSKOKAR - GIPUZKOA                      |\n";
    std::cout << " |   * disponible   X sin vehiculos / averiada             |\n";
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


void dibujar_minimapa(const std::string &nombre_estacion,
                      const std::vector<DatoVehiculo> &vehiculos,
                      int id_vehiculo_usuario) {

    /* --- Construir celdas de vehiculos --- */
    int disp = 0, av = 0, en_uso = 0;
    std::vector<std::string> celdas;

    for (const auto &v : vehiculos) {
        std::string c;
        if (v.id == id_vehiculo_usuario) {
            c = "[*" + std::to_string(v.id) + "*]";
        } else if (v.estado == "disponible") {
            c = "[ " + std::to_string(v.id) + " ]";
            disp++;
        } else if (v.estado == "averiado") {
            c = "[X" + std::to_string(v.id) + "X]";
            av++;
        } else { /* en_uso */
            c = "[~" + std::to_string(v.id) + "~]";
            en_uso++;
        }
        celdas.push_back(c);
    }

    /* --- Calcular ancho necesario --- */
    // Minimo 36; crece si hay muchos vehiculos o el nombre es largo
    int ancho_vehiculos = 0;
    for (const auto &c : celdas) ancho_vehiculos += (int)c.size() + 1;
    if (ancho_vehiculos > 0) ancho_vehiculos--; // quitar espacio final

    std::string titulo = " Estacion: " + nombre_estacion + " ";
    std::string leyenda  = " [V]=libre [~]=uso [X]=averiado [*]=tuyo ";
    std::string stats_s  = " Disp:" + std::to_string(disp)
                         + " En uso:" + std::to_string(en_uso)
                         + " Averiados:" + std::to_string(av) + " ";

    int ancho = 36;
    for (const auto *s : {&titulo, &leyenda, &stats_s})
        if ((int)s->size() > ancho) ancho = (int)s->size();
    if (ancho_vehiculos > ancho) ancho = ancho_vehiculos;

    /* --- Helpers de impresion --- */
    auto borde = [&](char c){ std::cout << " +" << std::string(ancho, c) << "+\n"; };

    // Imprime una linea con bordes | y relleno a la derecha
    auto linea_izq = [&](const std::string &texto) {
        int pad = ancho - (int)texto.size();
        if (pad < 0) pad = 0;
        std::cout << " |" << texto << std::string(pad, ' ') << "|\n";
    };

    // Imprime texto centrado entre bordes |
    auto linea_centro = [&](const std::string &texto) {
        int pad_total = ancho - (int)texto.size();
        int pad_l = pad_total / 2;
        int pad_r = pad_total - pad_l;
        if (pad_l < 0) pad_l = 0;
        if (pad_r < 0) pad_r = 0;
        std::cout << " |" << std::string(pad_l, ' ') << texto
                  << std::string(pad_r, ' ') << "|\n";
    };

    /* --- Dibujar marco --- */
    std::cout << "\n";
    borde('=');
    linea_centro(titulo);
    borde('-');

    if (celdas.empty()) {
        linea_centro("(Sin vehiculos en esta estacion)");
    } else {
        // Agrupar en filas de maximo 8 vehiculos
        const int POR_FILA = 8;
        for (int i = 0; i < (int)celdas.size(); i += POR_FILA) {
            std::string fila;
            for (int j = i; j < (int)celdas.size() && j < i + POR_FILA; ++j) {
                if (j > i) fila += ' ';
                fila += celdas[j];
            }
            // Ajustar ancho si esta fila es mas larga (caso borde calculado mal)
            linea_centro(fila);
        }
    }

    borde('-');
    linea_izq(leyenda);
    linea_izq(stats_s);
    borde('=');
    std::cout << "\n";
}