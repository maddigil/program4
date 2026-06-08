// Punto de entrada del CLIENTE C++ de Euskokar.

#include <iostream>
#include <fstream>
#include <string>
#include <limits>

#include "cliente.h"
#include "menu_usuario.h"
#include "protocolo.h"
using namespace std;
/* Lee una clave=valor de un fichero de configuracion simple */
static string leerConfig(const string &fichero,
                               const string &clave,
                               const string &defecto) {
    ifstream f(fichero);
    if (!f) return defecto;
    string linea;
    while (getline(f, linea)) {
        if (linea.empty() || linea[0] == '#') continue;
        size_t pos = linea.find('=');
        if (pos == string::npos) continue;
        string k = linea.substr(0, pos);
        string v = linea.substr(pos + 1);
        /* Quitar espacios */
        while (!k.empty() && k.back() == ' ') k.pop_back();
        while (!v.empty() && v.front() == ' ') v = v.substr(1);
        if (k == clave) return v;
    }
    return defecto;
}

int main() {
    cout << "\n";
    cout << " +================================+\n";
    cout << " |        EUSKOKAR  v1.0          |\n";
    cout << " |   Por un mundo mas sostenible  |\n";
    cout << " +================================+\n\n";

    //Leer IP y puerto del servidor desde config.cfg
    string ip     = leerConfig("datos/config.cfg", "servidor_ip",    "127.0.0.1");
    string puerto_s = leerConfig("datos/config.cfg", "servidor_puerto",
                                      to_string(PUERTO_DEFAULT));
    int puerto = stoi(puerto_s);

    cout << " Conectando a " << ip << ":" << puerto << "...\n";

    //Crear cliente y conectar
    Cliente cli;
    if (!cli.conectar(ip, puerto)) {
        cerr << "\n ERROR: No se pudo conectar al servidor.\n";
        cerr << " Asegurate de que el servidor esta en marcha.\n\n";
        return 1;
    }

    cout << " Conexion establecida.\n\n";

    //Login (hasta 3 intentos)
    int intentos = 3;
    bool logueado = false;
    int  id_usuario = -1;
    string nombre_usuario;

    while (intentos-- > 0 && !logueado) {
        string nombre, clave;

        cout << " Usuario : ";
        getline(cin, nombre);
        cout << " Contrasena: ";
        getline(cin, clave);

        //Enviar LOGIN 
        cli.enviarComando(string(CMD_LOGIN) + " " + nombre + "|" + clave);
        string resp = cli.leerLinea();

        if (Cliente::esOk(resp)) {
            string valor = Cliente::valorOk(resp);
            size_t espacio = valor.find(' ');
            if (espacio != string::npos) {
                id_usuario     = stoi(valor.substr(0, espacio));
                nombre_usuario = valor.substr(espacio + 1);
            } else {
                id_usuario     = stoi(valor);
                nombre_usuario = nombre;
            }
            logueado = true;
            cout << "\n Bienvenida/o, " << nombre_usuario << "!\n";
        } else {
            cout << "\n " << Cliente::mensajeError(resp);
            if (intentos > 0)
                cout << "  (" << intentos << " intentos restantes)\n\n";
        }
    }

    if (!logueado) {
        cout << "\n Demasiados intentos fallidos. Saliendo.\n\n";
        return 1;
    }

    //Menu principal del usuario
    menu_principal(cli, id_usuario, nombre_usuario);
    

    cout << "\nHasta pronto!\n";

  

   

    return 0;
}

