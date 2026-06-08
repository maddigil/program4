
#include "menu_usuario.h"
#include "cache_manager.h"
#include "ascii_mapa.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>


static std::vector<std::string> split(const std::string& linea, char sep = '|') {
    std::vector<std::string> campos;
    std::stringstream ss(linea);
    std::string campo;
    while (std::getline(ss, campo, sep)) campos.push_back(campo);
    return campos;
}

static EstacionCache parseEstacion(const std::string& linea) {
    auto f = split(linea);
    EstacionCache e{};
    if (f.size() >= 5) {
        e.id              = std::stoi(f[0]);
        e.codigo          = f[1];
        e.nombre          = f[2];
        e.total_vehiculos = std::stoi(f[3]);
        e.disponibles     = std::stoi(f[4]);
    }
    return e;
}

static VehiculoCache parseVehiculo(const std::string& linea, int id_vehiculo_activo) {
    auto f = split(linea);
    VehiculoCache v{};
    if (f.size() >= 3) {
        v.id          = std::stoi(f[0]);
        v.estado      = f[1];
        v.matricula   = "";
        v.id_estacion = 0;
        v.es_mio      = (v.id == id_vehiculo_activo);
    }
    return v;
}


MenuUsuario::MenuUsuario(Cliente& cli, int id_usuario,
                         const std::string& nombre_usuario)
    : cli_(cli),
      id_usuario_(id_usuario),
      nombre_usuario_(nombre_usuario),
      id_vehiculo_activo_(-1),
      id_trayecto_activo_(-1),
      cache_(std::make_unique<CacheManager>())
{}

MenuUsuario::~MenuUsuario() = default;


void MenuUsuario::ejecutar() {
    while (true) {
        mostrarMenu();
        int opcion = leerOpcion();
        switch (opcion) {
            case 1: opcionMapa();           break;
            case 2: opcionEstacion();       break;
            case 3: opcionReservar();       break;
            case 4: opcionUsarVehiculo();   break;
            case 5: opcionFinTrayecto();    break;
            case 6: opcionReportarAveria(); break;
            case 7: opcionHistorial();      break;
            case 0:
                cli_.enviarComando("SALIR");
                cli_.leerLinea();
                std::cout << "\nHasta pronto!\n";
                return;
            default:
                std::cout << "Opcion no valida.\n";
        }
    }
}


void MenuUsuario::mostrarMenu() const {
    std::cout << "\n+=================================+\n";
    std::cout << "|  EUSKOKAR - Menu usuario        |\n";
    std::cout << "|  Sesion: " << nombre_usuario_;
    int pad = 22 - (int)nombre_usuario_.size();
    for (int i = 0; i < pad; ++i) std::cout << ' ';
    std::cout << "|\n";
    std::cout << "+=================================+\n";
    std::cout << " 1. Ver mapa de Gipuzkoa\n";
    std::cout << " 2. Ver estacion concreta (minimapa)\n";
    std::cout << " 3. Reservar un vehiculo\n";
    std::cout << " 4. Desbloquear / iniciar trayecto\n";
    std::cout << " 5. Finalizar trayecto activo\n";
    std::cout << " 6. Reportar averia\n";
    std::cout << " 7. Ver mi historial de trayectos\n";
    std::cout << " 0. Salir\n";
    std::cout << "Opcion: ";
}

int MenuUsuario::leerOpcion() const {
    int op;
    std::cin >> op;
    std::cin.ignore();
    return op;
}


void MenuUsuario::opcionMapa() {
    std::vector<EstacionCache> estaciones;

    if (cache_->estacionesValidas()) {
        std::cout << "[cache] Usando datos en cache...\n";
        estaciones = cache_->getEstaciones();
    } else {
        std::cout << "Consultando servidor...\n";
        cli_.enviarComando("LISTAR_EST");
        std::vector<std::string> lineas = cli_.leerLista();
        if (lineas.size() == 1 && lineas[0].rfind("ERROR", 0) == 0) {
            std::cout << lineas[0] << "\n"; return;
        }
        for (const auto& l : lineas)
            if (!l.empty()) estaciones.push_back(parseEstacion(l));
        cache_->actualizarEstaciones(estaciones);
    }

    std::vector<DatoEstacion> datos;
    for (const auto& e : estaciones) {
        DatoEstacion d;
        d.id          = e.id;
        d.abrev       = e.codigo;
        d.nombre      = e.nombre;
        d.plazas      = e.total_vehiculos;
        d.disponibles = e.disponibles;
        datos.push_back(d);
    }
    dibujar_mapa_grande(datos, id_vehiculo_activo_ > 0 ? id_vehiculo_activo_ : 0);
}


void MenuUsuario::opcionEstacion() {
    std::vector<EstacionCache> estaciones;
    if (cache_->estacionesValidas()) {
        estaciones = cache_->getEstaciones();
    } else {
        std::cout << "Consultando estaciones...\n";
        cli_.enviarComando("LISTAR_EST");
        std::vector<std::string> lineas = cli_.leerLista();
        if (lineas.size() == 1 && lineas[0].rfind("ERROR", 0) == 0) {
            std::cout << lineas[0] << "\n"; return;
        }
        for (const auto& l : lineas)
            if (!l.empty()) estaciones.push_back(parseEstacion(l));
        cache_->actualizarEstaciones(estaciones);
    }

    std::cout << "\nEstaciones disponibles:\n";
    for (const auto& e : estaciones)
        std::cout << "  " << e.id << ". " << e.nombre
                  << " [" << e.disponibles << "/" << e.total_vehiculos << " libres]\n";

    std::cout << "ID de estacion (0 para volver): ";
    int id_est;
    std::cin >> id_est;
    std::cin.ignore();
    if (id_est <= 0) return;

    std::vector<VehiculoCache> vehiculos;
    if (cache_->vehiculosValidos(id_est)) {
        std::cout << "[cache] Usando vehiculos en cache...\n";
        vehiculos = cache_->getVehiculos(id_est);
    } else {
        std::cout << "Consultando vehiculos...\n";
        cli_.enviarComando("VEH_ESTACION " + std::to_string(id_est));
        std::vector<std::string> lineas = cli_.leerLista();
        if (lineas.size() == 1 && lineas[0].rfind("ERROR", 0) == 0) {
            std::cout << lineas[0] << "\n"; return;
        }
        for (const auto& l : lineas)
            if (!l.empty())
                vehiculos.push_back(parseVehiculo(l, id_vehiculo_activo_));
        cache_->actualizarVehiculos(id_est, vehiculos);
    }

    std::string nombre_est = "Estacion " + std::to_string(id_est);
    for (const auto& e : estaciones)
        if (e.id == id_est) { nombre_est = e.nombre; break; }

    std::vector<DatoVehiculo> datos;
    for (const auto& v : vehiculos) {
        DatoVehiculo d;
        d.id      = v.id;
        d.estado  = v.estado;
        d.bateria = 0.0f;
        datos.push_back(d);
    }
    dibujar_minimapa(nombre_est, datos,
                     id_vehiculo_activo_ > 0 ? id_vehiculo_activo_ : 0);
}


void MenuUsuario::opcionReservar() {
    std::cout << "ID del vehiculo a reservar: ";
    int id_veh;
    std::cin >> id_veh;
    std::cin.ignore();

    cli_.enviarComando("RESERVAR " + std::to_string(id_veh));
    std::string resp = cli_.leerLinea();
    std::cout << resp << "\n";

    if (resp.rfind("OK", 0) == 0)
        cache_->invalidarTodo();
}


void MenuUsuario::opcionUsarVehiculo() {
    if (id_vehiculo_activo_ != -1) {
        std::cout << "Ya tienes un vehiculo activo (ID " << id_vehiculo_activo_
                  << "). Finaliza el trayecto primero (opcion 5).\n";
        return;
    }
    std::cout << "ID del vehiculo a desbloquear: ";
    int id_veh;
    std::cin >> id_veh;
    std::cin.ignore();

    cli_.enviarComando("USAR_VEH " + std::to_string(id_veh));
    std::string resp = cli_.leerLinea();
    std::cout << resp << "\n";

    if (resp.rfind("OK", 0) == 0) {
        std::istringstream ss(resp);
        std::string ok;
        ss >> ok >> id_trayecto_activo_;
        id_vehiculo_activo_ = id_veh;
        cache_->invalidarTodo();
        std::cout << "Trayecto iniciado (ID " << id_trayecto_activo_ << "). Buen viaje!\n";
    }
}


void MenuUsuario::opcionFinTrayecto() {
    if (id_trayecto_activo_ == -1) {
        std::cout << "No tienes ningun trayecto activo.\n";
        return;
    }
    std::cout << "Distancia recorrida (km): ";
    double dist;
    std::cin >> dist;
    std::cin.ignore();

    cli_.enviarComando("FIN_TRAYECTO " + std::to_string(id_trayecto_activo_)
                       + " " + std::to_string(dist));
    std::string resp = cli_.leerLinea();
    std::cout << resp << "\n";

    if (resp.rfind("OK", 0) == 0) {
        id_vehiculo_activo_ = -1;
        id_trayecto_activo_ = -1;
        cache_->invalidarTodo();
    }
}


void MenuUsuario::opcionReportarAveria() {
    std::cout << "ID del vehiculo averiado: ";
    int id_veh;
    std::cin >> id_veh;
    std::cin.ignore();

    std::cout << "Tipo de averia (ej: neumatico, motor, frenos): ";
    std::string tipo;
    std::getline(std::cin, tipo);

    std::cout << "Descripcion: ";
    std::string desc;
    std::getline(std::cin, desc);

    for (auto& c : tipo) if (c == ' ') c = '_';
    for (auto& c : desc) if (c == ' ') c = '_';

    cli_.enviarComando("REPORTAR_AV " + std::to_string(id_veh)
                       + " " + tipo + " " + desc);
    std::string resp = cli_.leerLinea();
    std::cout << resp << "\n";

    if (resp.rfind("OK", 0) == 0)
        cache_->invalidarTodo();
}


void MenuUsuario::opcionHistorial() {
    cli_.enviarComando("HISTORIAL");
    std::vector<std::string> lineas = cli_.leerLista();
    if (lineas.size() == 1 && lineas[0].rfind("ERROR", 0) == 0) {
        std::cout << lineas[0] << "\n"; return;
    }

    std::cout << "\n--- Historial de trayectos (ultimos 20) ---\n";
    std::cout << std::left;
    std::cout.width(6);  std::cout << "ID";
    std::cout.width(20); std::cout << "Inicio";
    std::cout.width(20); std::cout << "Fin";
    std::cout.width(10); std::cout << "Km";
    std::cout << "\n" << std::string(56, '-') << "\n";

    for (const auto& l : lineas) {
        if (l.empty()) continue;
        auto f = split(l);
        if (f.size() >= 4) {
            std::cout.width(6);  std::cout << f[0];
            std::cout.width(20); std::cout << f[1];
            std::cout.width(20); std::cout << f[2];
            std::cout.width(10); std::cout << f[3];
            std::cout << "\n";
        } else {
            std::cout << l << "\n";
        }
    }
}
