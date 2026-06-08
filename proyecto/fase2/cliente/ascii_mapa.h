//Simbolos usados:
//  [V] → vehiculo disponible
// [*V*]→ vehiculo en uso por el usuario actual
//  [X] → vehiculo averiado
//  ●   → estacion con vehiculos disponibles (mapa grande)
//  *   → estacion con el vehiculo activo del usuario
// X   → estacion sin vehiculos o averiada

#ifndef ASCII_MAPA_H
#define ASCII_MAPA_H

#include <string>
#include <vector>

// Datos de estacion
struct DatoEstacion {
    int         id;
    std::string abrev;
    std::string nombre;
    int         plazas;
    int         disponibles;
};

//Datos de vehiculo
struct DatoVehiculo {
    int         id;
    //"disponible", "en_uso", "averiado"
    std::string estado;   
    float       bateria;
};


void dibujar_mapa_grande(const std::vector<DatoEstacion> &estaciones,
                          int id_vehiculo_usuario = 0);


void dibujar_minimapa(const std::string &nombre_estacion,
                      const std::vector<DatoVehiculo> &vehiculos,
                      int id_vehiculo_usuario = 0);

#endif 
