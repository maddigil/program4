#ifndef ADMIN_H_
#define ADMIN_H_

#include "sqlite3.h"
#include "config.h"

/* Muestra el login en pantalla y pide usuario y contraseña al usuario. */
int admin_login(const Config *cfg);

/* Muestra el menú principal del administrador. */
void admin_menu(sqlite3 *db, const Config *cfg);


#endif