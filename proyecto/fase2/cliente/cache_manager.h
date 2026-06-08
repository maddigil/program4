#pragma once

#include <string>
#include <vector>
#include <chrono>
using namespace std;

struct EstacionCache {
    int    id;
    string codigo;      
    string nombre;     
    int    total_vehiculos;
    int    disponibles;
};

struct VehiculoCache {
    int    id;
    string matricula;
    // "libre", "en_uso", "averiado", "reservado"
    string estado;      
    int    id_estacion;
    // true si es el vehiculo activo del usuario
    bool   es_mio;           
};


class CacheManager {
public:
    static constexpr int TTL_SEGUNDOS = 30;

    CacheManager();


    bool estacionesValidas() const;

    void actualizarEstaciones(const vector<EstacionCache>& lista);

    const vector<EstacionCache>& getEstaciones() const;

    void invalidarEstaciones();


    bool vehiculosValidos(int id_estacion) const;

    void actualizarVehiculos(int id_estacion,
                             const vector<VehiculoCache>& lista);

    vector<VehiculoCache> getVehiculos(int id_estacion) const;

    void invalidarVehiculos(int id_estacion);

    void invalidarTodo();

private:
    using Reloj    = chrono::steady_clock;
    using Instante = chrono::time_point<Reloj>;

    vector<EstacionCache> estaciones_;
    Instante                   ts_estaciones_;
    bool                       est_cargadas_ = false;

    struct EntradaVehiculos {
        int                       id_estacion;
        vector<VehiculoCache> vehiculos;
        Instante                   timestamp;
    };
    vector<EntradaVehiculos> cache_vehiculos_;

    bool haExpirado(const Instante& ts) const;
    EntradaVehiculos* buscarEntrada(int id_estacion);
    const EntradaVehiculos* buscarEntrada(int id_estacion) const;
};
