#include "protocolo.h"

#include <sstream>
#include <stdexcept>
#include <cstring>
#include <iomanip>

using std::istringstream;
using std::ostringstream;
using std::string;
using std::vector;

static int toInt(const string &s)
{
    try
    {
        return std::stoi(s);
    }
    catch (...)
    {
        return 0;
    }
}
static float toFloat(const string &s)
{
    try
    {
        return std::stof(s);
    }
    catch (...)
    {
        return 0.0f;
    }
}
vector<string> Protocolo::split(const string &s, char sep)
{
    vector<string> partes;
    istringstream ss(s);
    string parte;
    while (std::getline(ss, parte, sep))
    {
        partes.push_back(parte);
    }
    return partes;
}
string Protocolo::serializarLogin(const string &nombre, const string &clave)
{
    return string(CMD_LOGIN) + " " + nombre + " " + clave;
}
string Protocolo::serializarListarEstaciones()
{
    return string(CMD_LISTAR_EST);
}

string Protocolo::serializarVehiculosEstacion(int id_estacion)
{
    return string(CMD_VEH_ESTACION) + " " + std::to_string(id_estacion);
}

string Protocolo::serializarListarVehiculos()
{
    return string(CMD_LISTAR_VEH);
}

string Protocolo::serializarReservar(int id_vehiculo)
{
    return string(CMD_RESERVAR) + " " + std::to_string(id_vehiculo);
}
string Protocolo::serializarUsarVehiculo(int id_vehiculo)
{
    return string(CMD_USAR_VEH) + " " + std::to_string(id_vehiculo);
}
string Protocolo::serializarFinTrayecto(int id_trayecto, float distancia)
{
    ostringstream oss;
    oss << CMD_FIN_TRAYECTO << " "
        << id_trayecto << " "
        << std::fixed << std::setprecision(1) << distancia;
    return oss.str();
}

string Protocolo::serializarReportarAveria(int id_vehiculo, const string &tipo, const string &descripcion)
{
    string desc = descripcion.empty() ? "sin_descripcion" : descripcion;
    return string(CMD_REPORTAR_AV) + " " + std::to_string(id_vehiculo) + " " + tipo + " " + desc;
}

string Protocolo::serializarHistorial()
{
    return string(CMD_HISTORIAL);
}

string Protocolo::serializarSalir()
{
    return string(CMD_SALIR);
}

Estacion Protocolo::deserializarEstacion(const string &linea)
{
    Estacion e;
    e.id_estacion = -1;
    e.nombre[0] = '\0';
    e.abreviacion[0] = '\0';
    e.plazas = 0;
    e.disponibles = 0;

    auto campos = split(linea, '|');
    if (campos.size() < 4)
        return e;

    e.id_estacion = toInt(campos[0]);
    strncpy(e.abreviacion, campos[1].c_str(), sizeof(e.abreviacion) - 1);
    e.abreviacion[sizeof(e.abreviacion) - 1] = '\0';
    strncpy(e.nombre, campos[2].c_str(), sizeof(e.nombre) - 1);
    e.nombre[sizeof(e.nombre) - 1] = '\0';
    e.plazas = toInt(campos[3]);
    e.disponibles = (campos.size() >= 5) ? toInt(campos[4]) : 0;

    return e;
}

Vehiculo Protocolo::deserializarVehiculo(const string &linea)
{
    Vehiculo v;
    v.id_vehiculo = -1;
    v.estado[0] = '\0';
    v.ubicacion_estacion = 0;
    v.bateria_restante = 0.0f;
    v.abrev_estacion[0] = '\0';

    auto campos = split(linea, '|');
    if (campos.size() < 3)
        return v;

    v.id_vehiculo = toInt(campos[0]);
    strncpy(v.estado, campos[1].c_str(), sizeof(v.estado) - 1);
    v.estado[sizeof(v.estado) - 1] = '\0';
    v.bateria_restante = toFloat(campos[2]);

    if (campos.size() >= 4)
    {
        strncpy(v.abrev_estacion, campos[3].c_str(), sizeof(v.abrev_estacion) - 1);
        v.abrev_estacion[sizeof(v.abrev_estacion) - 1] = '\0';
    }

    return v;
}

Trayecto Protocolo::deserializarTrayecto(const string &linea)
{
    Trayecto t;
    t.id_trayecto = -1;
    t.usuario_id = 0;
    t.vehiculo_id = 0;
    t.inicio[0] = '\0';
    t.fin[0] = '\0';
    t.distancia = 0.0f;

    auto campos = split(linea, '|');
    if (campos.size() < 5)
        return t;

    t.id_trayecto = toInt(campos[0]);
    t.vehiculo_id = toInt(campos[1]);
    strncpy(t.inicio, campos[2].c_str(), sizeof(t.inicio) - 1);
    t.inicio[sizeof(t.inicio) - 1] = '\0';
    strncpy(t.fin, campos[3].c_str(), sizeof(t.fin) - 1);
    t.fin[sizeof(t.fin) - 1] = '\0';
    t.distancia = toFloat(campos[4]);

    return t;
}

bool Protocolo::esOk(const string &linea)
{
    if (linea.size() < 2)
        return false;
    if (linea.substr(0, 2) != "OK")
        return false;
    return linea.size() == 2 || linea[2] == ' ';
}

bool Protocolo::esError(const string &linea)
{
    return linea.rfind("ERROR", 0) == 0;
}

string Protocolo::extraerValorOk(const string &linea)
{
    if (linea.size() > 3)
        return linea.substr(3);
    return "";
}

string Protocolo::extraerMensajeError(const string &linea)
{
    if (linea.size() > 6)
        return linea.substr(6);
    return "";
}

bool Protocolo::parsearRespuestaLogin(const string &resp,
                                      int &id_out,
                                      string &nombre_out)
{
    if (!esOk(resp))
        return false;

    string valor = extraerValorOk(resp);
    if (valor.empty())
        return false;

    size_t pos = valor.find(' ');
    if (pos == string::npos)
    {
        id_out = toInt(valor);
        nombre_out = "";
        return id_out > 0;
    }

    id_out = toInt(valor.substr(0, pos));
    nombre_out = valor.substr(pos + 1);
    return id_out > 0;
}

int Protocolo::parsearIdOk(const string &resp)
{
    if (!esOk(resp))
        return -1;
    return toInt(extraerValorOk(resp));
}