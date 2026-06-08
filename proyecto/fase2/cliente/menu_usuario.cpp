
#include "menu_usuario.h"
#include "cache_manager.h"
#include "ascii_mapa.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
using namespace std;

static vector<std::string> split(const string& linea, char sep = '|') {
    vector<std::string> campos;
    stringstream ss(linea);
    string campo;
    while (getline(ss, campo, sep)) campos.push_back(campo);
    return campos;
}

static EstacionCache parseEstacion(const string& linea) {
    auto f = split(linea);
    EstacionCache e{};
    if (f.size() >= 5) {
        e.id              = stoi(f[0]);
        e.codigo          = f[1];
        e.nombre          = f[2];
        e.total_vehiculos = stoi(f[3]);
        e.disponibles     = stoi(f[4]);
    }
    return e;
}

static VehiculoCache parseVehiculo(const string& linea, int id_vehiculo_activo) {
    auto f = split(linea);
    VehiculoCache v{};
    if (f.size() >= 3) {
        v.id          = stoi(f[0]);
        v.estado      = f[1];
        v.matricula   = "";
        v.id_estacion = 0;
        v.es_mio      = (v.id == id_vehiculo_activo);
    }
    return v;
}


MenuUsuario::MenuUsuario(Cliente& cli, int id_usuario,
                         const string& nombre_usuario)
    : cli_(cli),
      id_usuario_(id_usuario),
      nombre_usuario_(nombre_usuario),
      id_vehiculo_activo_(-1),
      id_trayecto_activo_(-1),
      cache_(make_unique<CacheManager>())
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
            return;
            default:
                cout << "Opcion no valida.\n";
        }
    }
}


void MenuUsuario::mostrarMenu() const {
    cout << "\n+=================================+\n";
    cout << "|  EUSKOKAR - Menu usuario        |\n";
    cout << "|  Sesion: " << nombre_usuario_;
    int pad = 22 - (int)nombre_usuario_.size();
    for (int i = 0; i < pad; ++i) cout << ' ';
    cout << "|\n";
    cout << "+=================================+\n";
    cout << " 1. Ver mapa de Gipuzkoa\n";
    cout << " 2. Ver estacion concreta (minimapa)\n";
    cout << " 3. Reservar un vehiculo\n";
    cout << " 4. Desbloquear / iniciar trayecto\n";
    cout << " 5. Finalizar trayecto activo\n";
    cout << " 6. Reportar averia\n";
    cout << " 7. Ver mi historial de trayectos\n";
    cout << " 0. Salir\n";
    cout << "Opcion: ";
    if (id_vehiculo_reservado_ != -1) {
        cout << " ! Tienes una reserva activa (Vehiculo: " << id_vehiculo_reservado_ << ")\n";
    }
}

int MenuUsuario::leerOpcion() const {
    int op;
    cin >> op;
    cin.ignore();
    return op;
}


void MenuUsuario::opcionMapa() {
    vector<EstacionCache> estaciones;

    if (cache_->estacionesValidas()) {
        cout << "[cache] Usando datos en cache...\n";
        estaciones = cache_->getEstaciones();
    } else {
        cout << "Consultando servidor...\n";
        cli_.enviarComando("LISTAR_EST");
        vector<string> lineas = cli_.leerLista();
        if (lineas.size() == 1 && lineas[0].rfind("ERROR", 0) == 0) {
            cout << lineas[0] << "\n"; return;
        }
        for (const auto& l : lineas)
            if (!l.empty()) estaciones.push_back(parseEstacion(l));
        cache_->actualizarEstaciones(estaciones);
    }

    vector<DatoEstacion> datos;
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
   vector<EstacionCache> estaciones;
    if (cache_->estacionesValidas()) {
        estaciones = cache_->getEstaciones();
    } else {
        cout << "Consultando estaciones...\n";
        cli_.enviarComando("LISTAR_EST");
        vector<string> lineas = cli_.leerLista();
        if (lineas.size() == 1 && lineas[0].rfind("ERROR", 0) == 0) {
            cout << lineas[0] << "\n"; return;
        }
        for (const auto& l : lineas)
            if (!l.empty()) estaciones.push_back(parseEstacion(l));
        cache_->actualizarEstaciones(estaciones);
    }

    cout << "\nEstaciones disponibles:\n";
    for (const auto& e : estaciones)
        cout << "  " << e.id << ". " << e.nombre
                  << " [" << e.disponibles << "/" << e.total_vehiculos << " libres]\n";

    cout << "ID de estacion (0 para volver): ";
    int id_est;
    cin >> id_est;
    cin.ignore();
    if (id_est <= 0) return;

    vector<VehiculoCache> vehiculos;
    if (cache_->vehiculosValidos(id_est)) {
        cout << "[cache] Usando vehiculos en cache...\n";
        vehiculos = cache_->getVehiculos(id_est);
    } else {
        cout << "Consultando vehiculos...\n";
        cli_.enviarComando("VEH_ESTACION " + to_string(id_est));
        vector<string> lineas = cli_.leerLista();
        if (lineas.size() == 1 && lineas[0].rfind("ERROR", 0) == 0) {
           cout << lineas[0] << "\n"; return;
        }
        for (const auto& l : lineas)
            if (!l.empty())
                vehiculos.push_back(parseVehiculo(l, id_vehiculo_activo_));
        cache_->actualizarVehiculos(id_est, vehiculos);
    }

    string nombre_est = "Estacion " + to_string(id_est);
    for (const auto& e : estaciones)
        if (e.id == id_est) { nombre_est = e.nombre; break; }

    vector<DatoVehiculo> datos;
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
    cout << "ID del vehiculo a reservar: ";
    int id_veh;
    cin >> id_veh;
    cin.ignore();

    cli_.enviarComando("RESERVAR " + to_string(id_veh));
    string resp = cli_.leerLinea();
    
    if (resp.rfind("OK", 0) == 0) {
        id_vehiculo_reservado_ = id_veh; // para guardar el estado
        cache_->invalidarTodo();
       cout << "Reserva confirmada para el vehiculo " << id_veh << "\n";
    } else {
        cout << "Error en reserva: " << resp << "\n";
    }
}


void MenuUsuario::opcionUsarVehiculo() {
    if (id_vehiculo_activo_ != -1) {
        cout << "Ya tienes un vehiculo activo (ID " << id_vehiculo_activo_
                  << "). Finaliza el trayecto primero (opcion 5).\n";
        return;
    }
    cout << "ID del vehiculo a desbloquear: ";
    int id_veh;
    cin >> id_veh;
    cin.ignore();

    cli_.enviarComando("USAR_VEH " + to_string(id_veh));
    string resp = cli_.leerLinea();
   cout << resp << "\n";

    if (resp.rfind("OK", 0) == 0) {
        istringstream ss(resp);
        string ok;
        ss >> ok >> id_trayecto_activo_;
        id_vehiculo_activo_ = id_veh;
        cache_->invalidarTodo();
        cout << "Trayecto iniciado (ID " << id_trayecto_activo_ << "). Buen viaje!\n";
    }
}


void MenuUsuario::opcionFinTrayecto() {
    if (id_trayecto_activo_ == -1) {
        cout << "No tienes ningun trayecto activo.\n";
        return;
    }
    cout << "Distancia recorrida (km): ";
    double dist;
    cin >> dist;
    cin.ignore();

    cli_.enviarComando("FIN_TRAYECTO " + to_string(id_trayecto_activo_)
                       + " " + to_string(dist));
    string resp = cli_.leerLinea();
    cout << resp << "\n";

    if (resp.rfind("OK", 0) == 0) {
        id_vehiculo_activo_ = -1;
        id_trayecto_activo_ = -1;
        cache_->invalidarTodo();
    }
}


void MenuUsuario::opcionReportarAveria() {
    cout << "ID del vehiculo averiado: ";
    int id_veh;
    cin >> id_veh;
    cin.ignore();

    cout << "Tipo de averia (ej: neumatico, motor, frenos): ";
    string tipo;
    getline(cin, tipo);

    cout << "Descripcion: ";
    string desc;
    getline(cin, desc);

    for (auto& c : tipo) if (c == ' ') c = '_';
    for (auto& c : desc) if (c == ' ') c = '_';

    cli_.enviarComando("REPORTAR_AV " + to_string(id_veh)
                       + " " + tipo + " " + desc);
    string resp = cli_.leerLinea();
    cout << resp << "\n";

    if (resp.rfind("OK", 0) == 0)
        cache_->invalidarTodo();
}


void MenuUsuario::opcionHistorial() {
    cli_.enviarComando("HISTORIAL");
    vector<string> lineas = cli_.leerLista();
    if (lineas.size() == 1 && lineas[0].rfind("ERROR", 0) == 0) {
        cout << lineas[0] << "\n"; return;
    }

    cout << "\n--- Historial de trayectos (ultimos 20) ---\n";
    cout << left;
    cout.width(6);  cout << "ID";
    cout.width(6);  cout << "Veh";
    cout.width(22); cout << "Inicio";
    cout.width(22); cout << "Fin";
    cout.width(8);  cout << "Km";
    cout << "\n" << string(64, '-') << "\n";

    for (const auto& l : lineas) {
        if (l.empty()) continue;
        auto f = split(l);
        if (f.size() >= 5) {
            cout.width(6);  cout << f[0];   // id_trayecto
            cout.width(6);  cout << f[1];   // vehiculo_id
            cout.width(22); cout << f[2];   // inicio
            cout.width(22); cout << f[3];   // fin
            cout.width(8);  cout << f[4];   // distancia
            cout << "\n";
        } else {
            cout << l << "\n";
        }
    }
}