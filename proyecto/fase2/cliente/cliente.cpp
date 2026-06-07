/*
 * cliente.cpp
 * Implementacion de la clase Cliente.
 * Gestiona la conexion TCP con el servidor.
 *
 * ¿Que hacen los sockets del lado cliente?
 *   1. socket()  → crear el "enchufe"
 *   2. connect() → llamar al servidor (darle la IP y el puerto)
 *   3. send()    → enviar datos (nuestros comandos)
 *   4. recv()    → recibir datos (respuestas del servidor)
 *   5. close()   → colgar
 */

#include "cliente.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* ------------------------------------------------------------------ */
/* Constructor / Destructor                                             */
/* ------------------------------------------------------------------ */

Cliente::Cliente() : m_fd(-1), m_conectado(false) {}

Cliente::~Cliente() {
    desconectar();
}

/* ------------------------------------------------------------------ */
/* Conexion                                                             */
/* ------------------------------------------------------------------ */

bool Cliente::conectar(const std::string &ip, int puerto) {
    /* 1. Crear el socket TCP */
    m_fd = socket(AF_INET,     /* IPv4  */
                  SOCK_STREAM, /* TCP   */
                  0);
    if (m_fd < 0) {
        std::cerr << "[CLIENTE] Error al crear el socket\n";
        return false;
    }

    /* 2. Rellenar la direccion del servidor */
    struct sockaddr_in dir;
    memset(&dir, 0, sizeof(dir));
    dir.sin_family = AF_INET;
    dir.sin_port   = htons(puerto);

    /* Convertir la IP de texto a binario */
    if (inet_pton(AF_INET, ip.c_str(), &dir.sin_addr) <= 0) {
        std::cerr << "[CLIENTE] IP invalida: " << ip << "\n";
        close(m_fd);
        m_fd = -1;
        return false;
    }

    /* 3. Conectar al servidor */
    if (connect(m_fd, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
        std::cerr << "[CLIENTE] No se pudo conectar a "
                  << ip << ":" << puerto << "\n";
        close(m_fd);
        m_fd = -1;
        return false;
    }

    m_conectado = true;

    /* Leer el mensaje de bienvenida del servidor */
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

/* ------------------------------------------------------------------ */
/* Envio y recepcion                                                    */
/* ------------------------------------------------------------------ */

bool Cliente::enviarComando(const std::string &cmd) {
    if (!m_conectado) return false;

    std::string msg = cmd + "\n";
    int enviados = send(m_fd, msg.c_str(), msg.size(), 0);
    return enviados > 0;
}

/*
 * leerLinea():
 * Lee del socket caracter a caracter hasta encontrar '\n'.
 * Usamos un buffer interno (m_buffer) para no perder datos
 * si recv() nos devuelve varias lineas de golpe.
 */
std::string Cliente::leerLinea() {
    std::string linea;

    while (true) {
        /* Buscar un \n en el buffer acumulado */
        size_t pos = m_buffer.find('\n');
        if (pos != std::string::npos) {
            linea = m_buffer.substr(0, pos);
            m_buffer = m_buffer.substr(pos + 1); /* Guardar el resto */
            return linea;
        }

        /* No hay \n todavia: recibir mas datos del servidor */
        char chunk[512];
        int n = recv(m_fd, chunk, sizeof(chunk) - 1, 0);
        if (n <= 0) {
            /* Servidor cerro la conexion */
            m_conectado = false;
            return "";
        }
        chunk[n] = '\0';
        m_buffer += chunk;
    }
}

std::vector<std::string> Cliente::leerLista() {
    std::vector<std::string> resultado;

    /* La primera linea ya tiene que ser "OK" (cabecera de lista) */
    std::string cabecera = leerLinea();
    if (cabecera != RESP_OK) {
        /* No era una lista, devolvemos la linea como unico elemento */
        resultado.push_back(cabecera);
        return resultado;
    }

    /* Leer lineas hasta "FIN" */
    while (true) {
        std::string linea = leerLinea();
        if (linea == RESP_FIN || linea.empty()) break;
        resultado.push_back(linea);
    }

    return resultado;
}

/* ------------------------------------------------------------------ */
/* Utilidades estaticas                                                 */
/* ------------------------------------------------------------------ */

bool Cliente::esOk(const std::string &linea) {
    return linea.rfind("OK", 0) == 0; /* empieza por "OK" */
}

std::string Cliente::valorOk(const std::string &linea) {
    if (linea.size() > 3)
        return linea.substr(3); /* quita "OK " */
    return "";
}

std::string Cliente::mensajeError(const std::string &linea) {
    if (linea.size() > 6)
        return linea.substr(6); /* quita "ERROR " */
    return linea;
}
