#pragma once

#include <string>
#include <vector>
#include <chrono>


struct EstacionCache {
    int    id;
    std::string codigo;      
    std::string nombre;     
    int    total_vehiculos;
    int    disponibles;
};

struct VehiculoCache {
    int    id;
    std::string matricula;
    std::string estado;      // "libre", "en_uso", "averiado", "reservado"
    int    id_estacion;
    bool   es_mio;           // true si es el vehiculo activo del usuario
};


class CacheManager {
public:
    static constexpr int TTL_SEGUNDOS = 30;

    CacheManager();


    bool estacionesValidas() const;

    void actualizarEstaciones(const std::vector<EstacionCache>& lista);

    const std::vector<EstacionCache>& getEstaciones() const;

    void invalidarEstaciones();


    bool vehiculosValidos(int id_estacion) const;

    void actualizarVehiculos(int id_estacion,
                             const std::vector<VehiculoCache>& lista);

    std::vector<VehiculoCache> getVehiculos(int id_estacion) const;

    void invalidarVehiculos(int id_estacion);

    void invalidarTodo();

private:
    using Reloj    = std::chrono::steady_clock;
    using Instante = std::chrono::time_point<Reloj>;

    std::vector<EstacionCache> estaciones_;
    Instante                   ts_estaciones_;
    bool                       est_cargadas_ = false;

    struct EntradaVehiculos {
        int                       id_estacion;
        std::vector<VehiculoCache> vehiculos;
        Instante                   timestamp;
    };
    std::vector<EntradaVehiculos> cache_vehiculos_;

    bool haExpirado(const Instante& ts) const;
    EntradaVehiculos* buscarEntrada(int id_estacion);
    const EntradaVehiculos* buscarEntrada(int id_estacion) const;
};
