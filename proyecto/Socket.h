#ifndef SOCKET_H
#define SOCKET_H
#include <string>
typedef int SOCKET;
const SOCKET INVALID_SOCKET =-1;

class Cliente
{
    private:
        SOCKET  sock;

    public:
        Cliente();
        ~Cliente();
        bool concetar(const std::string& ip, int puerto);
        bool enviar(const std::string& mensaje);
        bool recibir(std::string& mensaje_out);
        void cerrar();
};

#endif