#include "cache_manager.h"
#include <algorithm>



CacheManager::CacheManager()
    : ts_estaciones_(Reloj::now()), est_cargadas_(false)
{}


bool CacheManager::haExpirado(const Instante& ts) const {
    auto ahora  = Reloj::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(ahora - ts).count();
    return elapsed >= TTL_SEGUNDOS;
}

CacheManager::EntradaVehiculos* CacheManager::buscarEntrada(int id_estacion) {
    for (auto& e : cache_vehiculos_) {
        if (e.id_estacion == id_estacion) return &e;
    }
    return nullptr;
}

const CacheManager::EntradaVehiculos* CacheManager::buscarEntrada(int id_estacion) const {
    for (const auto& e : cache_vehiculos_) {
        if (e.id_estacion == id_estacion) return &e;
    }
    return nullptr;
}


bool CacheManager::estacionesValidas() const {
    return est_cargadas_ && !haExpirado(ts_estaciones_);
}

void CacheManager::actualizarEstaciones(const std::vector<EstacionCache>& lista) {
    estaciones_    = lista;
    ts_estaciones_ = Reloj::now();
    est_cargadas_  = true;
}

const std::vector<EstacionCache>& CacheManager::getEstaciones() const {
    return estaciones_;
}

void CacheManager::invalidarEstaciones() {
    est_cargadas_ = false;
    estaciones_.clear();
}


bool CacheManager::vehiculosValidos(int id_estacion) const {
    const EntradaVehiculos* e = buscarEntrada(id_estacion);
    if (!e) return false;
    return !haExpirado(e->timestamp);
}

void CacheManager::actualizarVehiculos(int id_estacion,
                                       const std::vector<VehiculoCache>& lista) {
    EntradaVehiculos* e = buscarEntrada(id_estacion);
    if (e) {
        e->vehiculos  = lista;
        e->timestamp  = Reloj::now();
    } else {
        EntradaVehiculos nueva;
        nueva.id_estacion = id_estacion;
        nueva.vehiculos   = lista;
        nueva.timestamp   = Reloj::now();
        cache_vehiculos_.push_back(nueva);
    }
}

std::vector<VehiculoCache> CacheManager::getVehiculos(int id_estacion) const {
    const EntradaVehiculos* e = buscarEntrada(id_estacion);
    if (e) return e->vehiculos;
    return {};
}

void CacheManager::invalidarVehiculos(int id_estacion) {
    cache_vehiculos_.erase(
        std::remove_if(cache_vehiculos_.begin(), cache_vehiculos_.end(),
            [id_estacion](const EntradaVehiculos& e) {
                return e.id_estacion == id_estacion;
            }),
        cache_vehiculos_.end()
    );
}

void CacheManager::invalidarTodo() {
    invalidarEstaciones();
    cache_vehiculos_.clear();
}
