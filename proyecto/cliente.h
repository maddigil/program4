
#ifndef CLIENTE_H
#define CLIENTE_H

#include <string>
#include <vector>
#include "protocolo.h"

class Cliente {
public:
    Cliente();
    ~Cliente();

    bool conectar(const std::string &ip, int puerto = PUERTO_DEFAULT);

    void desconectar();

    bool conectado() const;

    bool enviarComando(const std::string &cmd);

    std::string leerLinea();

    std::vector<std::string> leerLista();

    static bool esOk(const std::string &linea);

    static std::string valorOk(const std::string &linea);

    static std::string mensajeError(const std::string &linea);

private:
    int  m_fd;          
    bool m_conectado;
    std::string m_buffer;
};

#endif 