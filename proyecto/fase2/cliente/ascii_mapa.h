//Simbolos usados:
//  [V] → vehiculo disponible
// [*V*]→ vehiculo en uso por el usuario actual
//  [X] → vehiculo averiado
//  ●   → estacion con vehiculos disponibles (mapa grande)
//  *   → estacion con el vehiculo activo del usuario
// X   → estacion sin vehiculos o averiada

#ifndef ASCII_MAPA_H
#define ASCII_MAPA_H
using namespace std;
#include <string>
#include <vector>

// Datos de estacion
struct DatoEstacion {
    int         id;
   string abrev;
    string nombre;
    int         plazas;
    int         disponibles;
};

//Datos de vehiculo
struct DatoVehiculo {
    int         id;
    //"disponible", "en_uso", "averiado"
    string estado;   
    float       bateria;
};


void dibujar_mapa_grande(const vector<DatoEstacion> &estaciones,
                          int id_vehiculo_usuario = 0);


void dibujar_minimapa(const string &nombre_estacion,
                      const vector<DatoVehiculo> &vehiculos,
                      int id_vehiculo_usuario = 0);

#endif 
