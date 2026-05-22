#include <iostream>
#include <winsock2.h>
#include "Socket.h"

using namespace std;


Servidor::Servidor() {
    
    WSAStartup(MAKEWORD(2,0), &WSAData);
    servidor=socket(AF_INET, SOCK_STREAM,0);
}

Servidor::~Servidor() {
    cerrar();
}

bool Servidor::escuchar(int puerto) {
    servidor = socket(AF_INET, SOCK_STREAM, 0);
    
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(puerto); 
    bind(servidor, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
    listen(servidor, 0);
    
    cout << "Escuchar en el puerto:  " << puerto << endl;
    return true;
}

   
    SOCKET Servidor::aceptar() 
    {
        int clienteAddrSize = sizeof(clienteAddr);
        
        if((cliente=accept(servidor , (SOCKADDR *)&clienteAddr, &clienteAddrSize)) != INVALID_SOCKET)
        {
            cout << "conectado"<<endl;
        }
        
    }

Cliente::Cliente() {
    WSAStartup(MAKEWORD(2,0), &WSAData);
    socket=INVALID_SOCKET;
}

Cliente::~Cliente() {
    cerrar();
}


bool Cliente::conectar(const std::string& ip, int puerto) {
   
    this->socket = ::socket(AF_INET, SOCK_STREAM, 0);
    
   
    if (this->socket == INVALID_SOCKET) {
        return false;
    }

    targetAddr.sin_family = AF_INET;
    targetAddr.sin_port = htons(puerto);
    targetAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    
    if (::connect(this->socket, (SOCKADDR*)&targetAddr, sizeof(targetAddr)) == SOCKET_ERROR) {
        std::cout << "No se ha conectado al servidor" << std::endl;
        cerrar();
        return false;
    }

    std::cout << "conectado al servidor" << std::endl;
    return true;
}