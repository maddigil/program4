//Clase C++ que gestiona la conexion TCP con el servidor Euskokar.

#ifndef CLIENTE_H
#define CLIENTE_H

#include <string>
#include <vector>
#include "protocolo.h"
using namespace std;
class Cliente {
public:
    Cliente();
    ~Cliente();

   
    bool conectar(const string &ip, int puerto = PUERTO_DEFAULT);

    void desconectar();

    bool conectado() const;

   
    bool enviarComando(const string &cmd);

  
    string leerLinea();

  
    vector<string> leerLista();

 
    static bool esOk(const string &linea);


    static string valorOk(const string &linea);

    static string mensajeError(const string &linea);

private:
    int  m_fd;          
    bool m_conectado;
    string m_buffer; 
};

#endif 
