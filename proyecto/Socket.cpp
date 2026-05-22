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
    servidor=INVALID_SOCKET;
}

Cliente::~Cliente() {
    cerrar();
}

bool Cliente::conectar(const std::string& ip, int puerto) {
    this->servidor = ::socket(AF_INET, SOCK_STREAM, 0);
    
    if (this->servidor == INVALID_SOCKET) {
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(puerto);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (::connect(this->servidor, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "No se ha conectado al servidor" << std::endl;
        cerrar();
        return false;
    }

    std::cout << "conectado al servidor" << std::endl;
    return true;
}

void Cliente::enviar() {
    cout << "Escirbir mensaje: ";
    cin>>this->buffer;
    send(servidor,buffer,sizeof(buffer),0);
    cout << "Enviado!" << endl;
    memset(buffer,0,sizeof(buffer));
}

void  Cliente::recibir(){
    recv(servidor,buffer,sizeof(buffer),0);
    cout << "El servidor dice: "<<buffer<<endl;
    memset(buffer,0,sizeof(buffer));
}