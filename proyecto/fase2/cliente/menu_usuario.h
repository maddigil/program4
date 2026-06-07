/*
 * menu_usuario.h
 * Menu interactivo del cliente C++ de Euskokar.
 * Contiene todas las opciones que puede realizar un usuario normal.
 */

#ifndef MENU_USUARIO_H
#define MENU_USUARIO_H

#include "cliente.h"

/*
 * menu_principal(cli, id_usuario, nombre)
 * Muestra el menu principal y gestiona todas las opciones del usuario.
 * cli      → conexion activa con el servidor
 * id_usuario → id del usuario tras el login
 * nombre     → nombre del usuario para mostrarlo en pantalla
 */
void menu_principal(Cliente &cli, int id_usuario, const std::string &nombre);

#endif /* MENU_USUARIO_H */
