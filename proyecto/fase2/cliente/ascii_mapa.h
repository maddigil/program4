/*
 * ascii_mapa.h
 * Visualizacion en ASCII del mapa de Gipuzkoa y de los minimapas
 * por estacion, tal y como se describe en el documento FASE_0.pdf.
 *
 * Simbolos usados:
 *   [V] → vehiculo disponible
 *  [*V*]→ vehiculo en uso por el usuario actual
 *   [X] → vehiculo averiado
 *   ●   → estacion con vehiculos disponibles (mapa grande)
 *   *   → estacion con el vehiculo activo del usuario
 *   X   → estacion sin vehiculos o averiada
 */

#ifndef ASCII_MAPA_H
#define ASCII_MAPA_H

#include <string>
#include <vector>

/* Datos de una estacion recibidos del servidor */
struct DatoEstacion {
    int         id;
    std::string abrev;
    std::string nombre;
    int         plazas;
    int         disponibles;
};

/* Datos de un vehiculo en una estacion */
struct DatoVehiculo {
    int         id;
    std::string estado;   /* "disponible", "en_uso", "averiado" */
    float       bateria;
};

/*
 * dibujar_mapa_grande(estaciones, id_vehiculo_usuario)
 * Muestra el mapa ASCII de toda Gipuzkoa con el estado de cada estacion.
 * id_vehiculo_usuario: id del vehiculo activo del usuario (0 si ninguno)
 */
void dibujar_mapa_grande(const std::vector<DatoEstacion> &estaciones,
                          int id_vehiculo_usuario = 0);

/*
 * dibujar_minimapa(nombre_estacion, vehiculos, id_vehiculo_usuario)
 * Muestra el minimapa de una estacion concreta con sus vehiculos.
 */
void dibujar_minimapa(const std::string &nombre_estacion,
                      const std::vector<DatoVehiculo> &vehiculos,
                      int id_vehiculo_usuario = 0);

#endif 
