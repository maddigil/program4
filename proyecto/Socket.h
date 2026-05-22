#ifndef SOCKET_H
#define SOCKET_H
#include <string>
#include <winsock2.h>

class Cliente
{
    private:
        WSADATA WSAData;
        SOCKET servidor;
        SOCKADDR_IN addr;
        char buffer[1024];
    public:
        Cliente();
        ~Cliente();
        bool conectar(const std::string& ip, int puerto); // para la ip y el puerto
        void enviar(); // esto por que el cliente pide cosas
        void recibir(); // esto es por que se va a quedar esperando el cliente hasta que el servidor le responda
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