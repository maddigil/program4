// Punto de entrada del CLIENTE C++ de Euskokar.

#include <iostream>
#include <fstream>
#include <string>
#include <limits>

#include "cliente.h"
#include "menu_usuario.h"
#include "protocolo.h"

/* Lee una clave=valor de un fichero de configuracion simple */
static std::string leerConfig(const std::string &fichero,
                               const std::string &clave,
                               const std::string &defecto) {
    std::ifstream f(fichero);
    if (!f) return defecto;
    std::string linea;
    while (std::getline(f, linea)) {
        if (linea.empty() || linea[0] == '#') continue;
        size_t pos = linea.find('=');
        if (pos == std::string::npos) continue;
        std::string k = linea.substr(0, pos);
        std::string v = linea.substr(pos + 1);
        /* Quitar espacios */
        while (!k.empty() && k.back() == ' ') k.pop_back();
        while (!v.empty() && v.front() == ' ') v = v.substr(1);
        if (k == clave) return v;
    }
    return defecto;
}

int main() {
    std::cout << "\n";
    std::cout << " +================================+\n";
    std::cout << " |        EUSKOKAR  v1.0          |\n";
    std::cout << " |   Por un mundo mas sostenible  |\n";
    std::cout << " +================================+\n\n";

    //Leer IP y puerto del servidor desde config.cfg
    std::string ip     = leerConfig("datos/config.cfg", "servidor_ip",    "127.0.0.1");
    std::string puerto_s = leerConfig("datos/config.cfg", "servidor_puerto",
                                      std::to_string(PUERTO_DEFAULT));
    int puerto = std::stoi(puerto_s);

    std::cout << " Conectando a " << ip << ":" << puerto << "...\n";

    //Crear cliente y conectar
    Cliente cli;
    if (!cli.conectar(ip, puerto)) {
        std::cerr << "\n ERROR: No se pudo conectar al servidor.\n";
        std::cerr << " Asegurate de que el servidor esta en marcha.\n\n";
        return 1;
    }

    std::cout << " Conexion establecida.\n\n";

    //Login (hasta 3 intentos)
    int intentos = 3;
    bool logueado = false;
    int  id_usuario = -1;
    std::string nombre_usuario;

    while (intentos-- > 0 && !logueado) {
        std::string nombre, clave;

        std::cout << " Usuario : ";
        std::getline(std::cin, nombre);
        std::cout << " Contrasena: ";
        std::getline(std::cin, clave);

        //Enviar LOGIN 
        cli.enviarComando(std::string(CMD_LOGIN) + " " + nombre + "|" + clave);
        std::string resp = cli.leerLinea();

        if (Cliente::esOk(resp)) {
            std::string valor = Cliente::valorOk(resp);
            size_t espacio = valor.find(' ');
            if (espacio != std::string::npos) {
                id_usuario     = std::stoi(valor.substr(0, espacio));
                nombre_usuario = valor.substr(espacio + 1);
            } else {
                id_usuario     = std::stoi(valor);
                nombre_usuario = nombre;
            }
            logueado = true;
            std::cout << "\n Bienvenida/o, " << nombre_usuario << "!\n";
        } else {
            std::cout << "\n " << Cliente::mensajeError(resp);
            if (intentos > 0)
                std::cout << "  (" << intentos << " intentos restantes)\n\n";
        }
    }

    if (!logueado) {
        std::cout << "\n Demasiados intentos fallidos. Saliendo.\n\n";
        return 1;
    }

    //Menu principal del usuario
    menu_principal(cli, id_usuario, nombre_usuario);

    menu_principal(cli, id_usuario, nombre_usuario);

    std::cout << "\n Hasta pronto.\n\n";
    
    return 0;
}
