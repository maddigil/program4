/*
 * cliente.h
 * Clase C++ que gestiona la conexion TCP con el servidor Euskokar.
 *
 * Encapsula toda la logica de sockets para que el resto del codigo
 * cliente solo tenga que llamar a metodos como enviarComando() o leerLinea().
 */

#ifndef CLIENTE_H
#define CLIENTE_H

#include <string>
#include <vector>
#include "protocolo.h"

class Cliente {
public:
    /* Constructor / Destructor */
    Cliente();
    ~Cliente();

    /*
     * conectar(ip, puerto)
     * Establece la conexion TCP con el servidor.
     * Devuelve true si se conecto bien, false si hubo error.
     */
    bool conectar(const std::string &ip, int puerto = PUERTO_DEFAULT);

    /* Cierra la conexion */
    void desconectar();

    /* Devuelve true si la conexion esta activa */
    bool conectado() const;

    /*
     * enviarComando(cmd)
     * Envia una linea de texto al servidor (añade \n automaticamente).
     */
    bool enviarComando(const std::string &cmd);

    /*
     * leerLinea()
     * Lee una linea de respuesta del servidor (hasta \n).
     * Bloquea hasta que llega la respuesta.
     */
    std::string leerLinea();

    /*
     * leerLista()
     * Lee lineas del servidor hasta recibir "FIN".
     * Devuelve un vector con todas las lineas recibidas.
     */
    std::vector<std::string> leerLista();

    /*
     * esOk(linea)
     * Devuelve true si la linea empieza por "OK".
     */
    static bool esOk(const std::string &linea);

    /*
     * valorOk(linea)
     * Extrae el valor despues de "OK " en una respuesta simple.
     * Por ejemplo: "OK 42 Juan" → "42 Juan"
     */
    static std::string valorOk(const std::string &linea);

    /*
     * mensajeError(linea)
     * Extrae el mensaje de error despues de "ERROR ".
     */
    static std::string mensajeError(const std::string &linea);

private:
    int  m_fd;          /* File descriptor del socket */
    bool m_conectado;
    std::string m_buffer; /* Buffer para acumular datos recibidos */
};

#endif /* CLIENTE_H */
