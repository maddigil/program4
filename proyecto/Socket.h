#ifndef SOCKET_H
#define SOCKET_H
#include <string>
#include <winsock2.h>
typedef int SOCKET;

class Cliente
{
    private:
        SOCKET  sock;

    public:
        Cliente();
        ~Cliente();
        bool concetar(const std::string& ip, int puerto); // para la ip y el puerto
        bool enviar(const std::string& mensaje); // esto por que el cliente pide cosas
        bool recibir(std::string& mensaje_out); // esto es por que se va a quedar esperando el cliente hasta que el servidor le responda
        void cerrar();
};


class Servidor
{
    private:
        SOCKET listening; // esto es para que el programa esta escuchando la red
         WSADATA WSAData;
        SOCKET servidor, cliente;
        SOCKADDR_IN serverAddr, clienteAddr;
        char buffer[1024];
    public:
        Servidor();
        ~Servidor();
        bool escuchar(int puerto); // esto es por si un cliente llama al puerto
        SOCKET aceptar(); //esto es para crear conexion con el cliente
        void cerrar();
};

#endif