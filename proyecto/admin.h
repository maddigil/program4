#ifndef ADMIN_H_
#define ADMIN_H_
#include "sqlite3.h"
#include "config.h"

/*muestra el login en pantalla y pide usuario y contraseña al usuario.*/
int admin_login(const Config *cfg); /*este const no permite modificar el valor de configuración*/

/*muestra el menú principal del administrador y hasta que no pulsa 0 (salir) no termina*/
void admin_menu(sqlite3 *db, const Config *cfg);


#endif