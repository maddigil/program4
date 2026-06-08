#include "ascii_mapa.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
using namespace std;

/* Utilidades internas del proyecto: */

/* Busca una estacion por abreviatura en el vector */
static const DatoEstacion *buscarEstacion(
        const vector<DatoEstacion> &ests,
        const string &abrev) {
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
static string simboloEstacion(const DatoEstacion *e) {
    if (!e) return "?";
    return (e->disponibles > 0) ? "*" : "X";
}

/* Formatea una celda del mapa grande: "(AB)*" */
static string celda(const vector<DatoEstacion> &ests,
                    const string &abrev) {
    const DatoEstacion *e = buscarEstacion(ests, abrev);
    string sim = simboloEstacion(e);
    return "(" + abrev + ")" + sim;
}

/* ------------------------------------------------------------------ */
/* Mapa grande de Gipuzkoa */
/* ------------------------------------------------------------------ */
void dibujar_mapa_grande(const vector<DatoEstacion> &estaciones,
                         int /*id_vehiculo_usuario*/) {
    cout << "\n";
    cout << " +=========================================================+\n";
    cout << " |           MAPA EUSKOKAR - GIPUZKOA                      |\n";
    cout << " |   * disponible   X sin vehiculos / averiada            |\n";
    cout << " +=========================================================+\n";
    cout << " |                                                         |\n";

    /* Fila 1: costa norte (Zarautz, Donostia, Errenteria, Hondarribia) */
    cout << " |  "
         << left << setw(8) << celda(estaciones, "Z")
         << setw(8) << celda(estaciones, "DC")
         << setw(8) << celda(estaciones, "DG")
         << setw(8) << celda(estaciones, "E")
         << setw(8) << celda(estaciones, "H")
         << "       |\n";

    /* Fila 2: zona costera interior */
    cout << " |  "
         << setw(8) << celda(estaciones, "DA")
         << setw(8) << celda(estaciones, "DA2")
         << setw(8) << celda(estaciones, "DL")
         << setw(8) << celda(estaciones, "P")
         << "               |\n";

    cout << " |                                                         |\n";

    /* Fila 3: zona interior norte */
    cout << " |     "
         << setw(8) << celda(estaciones, "U")
         << setw(8) << celda(estaciones, "T")
         << setw(8) << celda(estaciones, "EB")
         << "                    |\n";

    /* Fila 4: zona interior */
    cout << " |     "
         << setw(8) << celda(estaciones, "A")
         << setw(8) << celda(estaciones, "L")
         << setw(8) << celda(estaciones, "HE")
         << setw(8) << celda(estaciones, "O")
         << "          |\n";

    /* Fila 5: sur */
    cout << " |        "
         << setw(8) << celda(estaciones, "B")
         << setw(8) << celda(estaciones, "S")
         << setw(8) << celda(estaciones, "M")
         << "               |\n";

    cout << " |                                                         |\n";
    cout << " +=========================================================+\n\n";

    /* Leyenda con conteos */
    int total_disp = 0, total_est = (int)estaciones.size();
    for (const auto &e : estaciones) total_disp += e.disponibles;
    cout << " Estaciones: " << total_est
         << "   Vehiculos disponibles: " << total_disp << "\n\n";
}

void dibujar_minimapa(const string &nombre_estacion,
                      const vector<DatoVehiculo> &vehiculos,
                      int id_vehiculo_usuario) {
    /* --- Construir celdas de vehiculos --- */
    int disp = 0, av = 0, en_uso = 0, res = 0;
    vector<string> celdas;

    for (const auto &v : vehiculos) {
        string c;
        if (v.id == id_vehiculo_usuario) {
            c = "[*" + to_string(v.id) + "*]";
        } else if (v.estado == "reservado") {
            c = "[#" + to_string(v.id) + "#]";
            res++;
        } else if (v.estado == "disponible") {
            c = "[ " + to_string(v.id) + " ]";
            disp++;
        } else if (v.estado == "averiado") {
            c = "[X" + to_string(v.id) + "X]";
            av++;
        } else { /* en_uso */
            c = "[~" + to_string(v.id) + "~]";
            en_uso++;
        }
        celdas.push_back(c);
    }

    /* --- Calcular ancho necesario --- */
    // Minimo 36; crece si hay muchos vehiculos o el nombre es largo
    int ancho_vehiculos = 0;
    for (const auto &c : celdas) ancho_vehiculos += (int)c.size() + 1;
    if (ancho_vehiculos > 0) ancho_vehiculos--; // quitar espacio final

    string titulo = " Estacion: " + nombre_estacion + " ";
    string leyenda  = " [#]=reservado [V]=libre [~]=uso [X]=averiado [*]=tuyo ";
    string stats_s  = " Disp:" + to_string(disp)
                    + " Reservados:" + to_string(res)
                    + " En uso:" + to_string(en_uso)
                    + " Averiados:" + to_string(av) + " ";

    int ancho = 36;
    for (const auto *s : {&titulo, &leyenda, &stats_s})
        if ((int)s->size() > ancho) ancho = (int)s->size();
    if (ancho_vehiculos > ancho) ancho = ancho_vehiculos;

    /* --- Helpers de impresion --- */
    auto borde = [&](char c){ cout << " +" << string(ancho, c) << "+\n"; };

    // Imprime una linea con bordes | y relleno a la derecha
    auto linea_izq = [&](const string &texto) {
        int pad = ancho - (int)texto.size();
        if (pad < 0) pad = 0;
        cout << " |" << texto << string(pad, ' ') << "|\n";
    };

    // Imprime texto centrado entre bordes |
    auto linea_centro = [&](const string &texto) {
        int pad_total = ancho - (int)texto.size();
        int pad_l = pad_total / 2;
        int pad_r = pad_total - pad_l;
        if (pad_l < 0) pad_l = 0;
        if (pad_r < 0) pad_r = 0;
        cout << " |" << string(pad_l, ' ') << texto
             << string(pad_r, ' ') << "|\n";
    };

    /* --- Dibujar marco --- */
    cout << "\n";
    borde('=');
    linea_centro(titulo);
    borde('-');

    if (celdas.empty()) {
        linea_centro("(Sin vehiculos en esta estacion)");
    } else {
        // Agrupar en filas de maximo 8 vehiculos
        const int POR_FILA = 8;
        for (int i = 0; i < (int)celdas.size(); i += POR_FILA) {
            string fila;
            for (int j = i; j < (int)celdas.size() && j < i + POR_FILA; ++j) {
                if (j > i) fila += ' ';
                fila += celdas[j];
            }
            linea_centro(fila);
        }
    }

    borde('-');
    linea_izq(leyenda);
    linea_izq(stats_s);
    borde('=');
    cout << "\n";
}