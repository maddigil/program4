#pragma once

#include "cliente.h"
#include <string>
#include <memory>
using namespace std;
class CacheManager;

class MenuUsuario {
public:
    MenuUsuario(Cliente& cli, int id_usuario, const string& nombre_usuario);
    ~MenuUsuario();
    void ejecutar();

private:
    Cliente&    cli_;
    int         id_usuario_;
    string nombre_usuario_;
    int         id_vehiculo_activo_;
    int         id_trayecto_activo_;
    unique_ptr<CacheManager> cache_;
    int id_vehiculo_reservado_;

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
                            const string& nombre) {
    MenuUsuario mu(cli, id_usuario, nombre);
    mu.ejecutar();
}
