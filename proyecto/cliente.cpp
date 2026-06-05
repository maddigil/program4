

#include "cliente.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
//hay que incluir estos tres pero dan error
#include <socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


Cliente::Cliente() : m_fd(-1), m_conectado(false) {}

Cliente::~Cliente() {
    desconectar();
}

bool Cliente::conectar(const std::string &ip, int puerto) {

    m_fd = socket(AF_INET,   
                  SOCK_STREAM, 
                  0);
    if (m_fd < 0) {
        std::cerr << "[CLIENTE] Error al crear el socket\n";
        return false;
    }


    struct sockaddr_in dir;
    memset(&dir, 0, sizeof(dir));
    dir.sin_family = AF_INET;
    dir.sin_port   = htons(puerto);


    if (inet_pton(AF_INET, ip.c_str(), &dir.sin_addr) <= 0) {
        std::cerr << "[CLIENTE] IP invalida: " << ip << "\n";
        close(m_fd);
        m_fd = -1;
        return false;
    }


    if (connect(m_fd, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
        std::cerr << "[CLIENTE] No se pudo conectar a "
                  << ip << ":" << puerto << "\n";
        close(m_fd);
        m_fd = -1;
        return false;
    }

    m_conectado = true;


    std::string bienvenida = leerLinea();
    std::cout << "[Servidor] " << bienvenida << "\n";

    return true;
}

void Cliente::desconectar() {
    if (m_fd >= 0) {
        close(m_fd);
        m_fd = -1;
    }
    m_conectado = false;
}

bool Cliente::conectado() const {
    return m_conectado;
}


bool Cliente::enviarComando(const std::string &cmd) {
    if (!m_conectado) return false;

    std::string msg = cmd + "\n";
    int enviados = send(m_fd, msg.c_str(), msg.size(), 0);
    return enviados > 0;
}


std::string Cliente::leerLinea() {
    std::string linea;

    while (true) {

        size_t pos = m_buffer.find('\n');
        if (pos != std::string::npos) {
            linea = m_buffer.substr(0, pos);
            m_buffer = m_buffer.substr(pos + 1); 
            return linea;
        }

        char chunk[512];
        int n = recv(m_fd, chunk, sizeof(chunk) - 1, 0);
        if (n <= 0) {
         
            m_conectado = false;
            return "";
        }
        chunk[n] = '\0';
        m_buffer += chunk;
    }
}

std::vector<std::string> Cliente::leerLista() {
    std::vector<std::string> resultado;


    std::string cabecera = leerLinea();
    if (cabecera != RESP_OK) {
   
        resultado.push_back(cabecera);
        return resultado;
    }


    while (true) {
        std::string linea = leerLinea();
        if (linea == RESP_FIN || linea.empty()) break;
        resultado.push_back(linea);
    }

    return resultado;
}


bool Cliente::esOk(const std::string &linea) {
    return linea.rfind("OK", 0) == 0; 
}

std::string Cliente::valorOk(const std::string &linea) {
    if (linea.size() > 3)
        return linea.substr(3); 
    return "";
}

std::string Cliente::mensajeError(const std::string &linea) {
    if (linea.size() > 6)
        return linea.substr(6);
    return linea;
}
cliente.cpp
Mostrando cliente.cpp.