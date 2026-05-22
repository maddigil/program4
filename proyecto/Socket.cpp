#include <iostream>
#include <winsock2.h>
#include "Socket.h"

using namespace std;


Servidor::Servidor() {
    
    WSAStartup(MAKEWORD(2,0), &WSAData);
    servidor=socket(AF_INET, SOCK_STREAM,0);
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
        
        if((cliente=accept(servidor , (SOCKADDR *)&clienteAddr, &clienteAddrSize))! = INVALID_SOCKET)
        {
            cout << "conectado"<<endl;
        }
        
    }
        