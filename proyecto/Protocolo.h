#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

#include <string>
#include <vector>

#define PUERTO_DEFAULT 8080
#define TAM_BUF 4096

#define CMD_LOGIN "LOGIN"
#define CMD_LISTAR_EST "LISTAR_EST"
#define CMD_VEH_ESTACION "VEH_ESTACION"
#define CMD_RESERVAR "RESERVAR"
#define CMD_USAR_VEH "USAR_VEH"
#define CMD_LISTAR_VEH "LISTAR_VEH"
#define CMD_FIN_TRAYECTO "FIN_TRAYECTO"
#define CMD_REPORTAR_AV "REPORTAR_AV"
#define CMD_HISTORIAL "HISTORIAL"
#define CMD_SALIR "SALIR"

#define RESP_OK "OK"
#define RESP_ERROR "ERROR"
#define RESP_FIN "FIN"

struct Estacion
{
    int id_estacion;
    char nombre[100];
    char abreviacion[10];
    int plazas;
    int disponibles;
};
struct Vehiculo
{
    int id_vehiculo;
    char estado[20];
    int ubicacion_estacion;
    float bateria_restante;
    char abrev_estacion[10];
};
struct Trayecto
{
    int id_trayecto;
    int usuario_id;
    int vehiculo_id;
    char inicio[30];
    char fin[30];
    float distancia;
};

class Protocolo
{
public:
    static std::vector<std::string> split(const std::string &s, char sep);

    static std::string serializarLogin(const std::string &nombre, const std::string &clave);
    static std::string serializarListarEstaciones();
    static std::string serializarVehiculosEstacion(int id_estacion);
    static std::string serializarListarVehiculos();
    static std::string serializarReservar(int id_vehiculo);
    static std::string serializarUsarVehiculo(int id_vehiculo);
    static std::string serializarFinTrayecto(int id_trayecto, float distancia);
    static std::string serializarReportarAveria(int id_vehiculo, const std::string &tipo, const std::string &descripcion);
    static std::string serializarHistorial();
    static std::string serializarSalir();

    static Estacion deserializarEstacion(const std::string &linea);
    static Vehiculo deserializarVehiculo(const std::string &linea);
    static Trayecto deserializarTrayecto(const std::string &linea);

    static bool esOk(const std::string &linea);
    static bool esError(const std::string &linea);

    static std::string extraerValorOk(const std::string &linea);
    static std::string extraerMensajeError(const std::string &linea);

    static bool parsearRespuestaLogin(const std::string &resp, int &id_out, std::string &nombre_out);
    static int parsearIdOk(const std::string &resp);
};

#endif
