#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <winsock2.h>
  #include <windows.h>
  #define close(s) closesocket(s)
#else
  #include <unistd.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
#endif

#include "cliente.h"
#include <iostream>
#include <cstring>
using namespace std;

Cliente::Cliente() : m_fd(-1), m_conectado(false) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif
}

Cliente::~Cliente() {
    desconectar();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool Cliente::conectar(const string &ip, int puerto) {
    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd < 0) {
        cerr << "[CLIENTE] Error al crear el socket\n";
        return false;
    }

    struct sockaddr_in dir;
    memset(&dir, 0, sizeof(dir));
    dir.sin_family = AF_INET;
    dir.sin_port   = htons(puerto);

    dir.sin_addr.s_addr = inet_addr(ip.c_str());
    if (dir.sin_addr.s_addr == INADDR_NONE) {
        cerr << "[CLIENTE] IP invalida: " << ip << "\n";
        close(m_fd);
        m_fd = -1;
        return false;
    }

    if (connect(m_fd, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
        cerr << "[CLIENTE] No se pudo conectar a "
                  << ip << ":" << puerto << "\n";
        close(m_fd);
        m_fd = -1;
        return false;
    }

    m_conectado = true;

    string bienvenida = leerLinea();
    cout << "[Servidor] " << bienvenida << "\n";

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

bool Cliente::enviarComando(const string &cmd) {
    if (!m_conectado) return false;
    string msg = cmd + "\n";
    int enviados = send(m_fd, msg.c_str(), (int)msg.size(), 0);
    return enviados > 0;
}

string Cliente::leerLinea() {
    string linea;

    while (true) {
        size_t pos = m_buffer.find('\n');
        if (pos != string::npos) {
            linea    = m_buffer.substr(0, pos);
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

vector<string> Cliente::leerLista() {
    vector<string> resultado;

    string cabecera = leerLinea();
    if (cabecera != RESP_OK) {
        resultado.push_back(cabecera);
        return resultado;
    }

    while (true) {
        string linea = leerLinea();
        if (linea == RESP_FIN || linea.empty()) break;
        resultado.push_back(linea);
    }

    return resultado;
}

bool Cliente::esOk(const string &linea) {
    return linea.rfind("OK", 0) == 0;
}

string Cliente::valorOk(const string &linea) {
    if (linea.size() > 3)
        return linea.substr(3);
    return "";
}

string Cliente::mensajeError(const string &linea) {
    if (linea.size() > 6)
        return linea.substr(6);
    return linea;
}
