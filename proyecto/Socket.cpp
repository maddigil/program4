#include <iostream>
#include <winsock2.h>
#include "Socket.h"

using namespace std;


Servidor::Servidor() {
    
    WSAStartup(MAKEWORD(2,0), &WSAData);
    servidor=socket(AF_INET, SOCK_STREAM,0);
}
