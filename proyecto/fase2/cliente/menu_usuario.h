#pragma once

#include "cliente.h"
#include <string>
#include <memory>

class CacheManager;

class MenuUsuario {
public:
    MenuUsuario(Cliente& cli, int id_usuario, const std::string& nombre_usuario);
    ~MenuUsuario();

    void ejecutar();

private:
    Cliente&    cli_;
    int         id_usuario_;
    std::string nombre_usuario_;
    int         id_vehiculo_activo_;   // -1 si no tiene vehiculo activo
    int         id_trayecto_activo_;   // -1 si no hay trayecto en curso

    std::unique_ptr<CacheManager> cache_;

    void mostrarMenu() const;
    int  leerOpcion()  const;

    void opcionMapa();
    void opcionEstacion();
    void opcionReservar();
    void opcionUsarVehiculo();
    void opcionFinTrayecto();
    void opcionReportarAveria();
    void opcionHistorial();
};
