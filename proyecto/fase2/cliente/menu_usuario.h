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
    int         id_vehiculo_activo_;
    int         id_trayecto_activo_;
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

inline void menu_principal(Cliente& cli, int id_usuario,
                            const std::string& nombre) {
    MenuUsuario mu(cli, id_usuario, nombre);
    mu.ejecutar();
}
