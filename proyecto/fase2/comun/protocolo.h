/*
 * protocolo.h
 * Constantes del protocolo de comunicacion cliente-servidor de Euskokar.
 * Este fichero lo usan TANTO el servidor (C) como el cliente (C++).
 *
 * Protocolo de texto simple:
 *   Cliente envia:  "COMANDO param1 param2\n"
 *   Servidor responde:
 *       - Dato unico:  "OK valor\n"
 *       - Lista:       "OK\nlinea1\nlinea2\n...\nFIN\n"
 *       - Error:       "ERROR mensaje\n"
 *
 * Separador de campos dentro de una linea de lista: '|'
 * Ejemplo lista estaciones:  "1|DC|Donostia Centro|10|3\n"
 *                             id|abrev|nombre|plazas|disponibles
 */

#ifndef PROTOCOLO_H
#define PROTOCOLO_H

/* --- Conexion --- */
#define PUERTO_DEFAULT  8080
#define TAM_BUF         4096    /* Tamaño del buffer de red */

/* --- Comandos (cliente → servidor) --- */

/* LOGIN nombre clave
   Respuesta: OK id_usuario nombre   o   ERROR mensaje */
#define CMD_LOGIN        "LOGIN"

/* LISTAR_EST
   Respuesta lista: id|abrev|nombre|plazas|disponibles */
#define CMD_LISTAR_EST   "LISTAR_EST"

/* VEH_ESTACION id_estacion
   Respuesta lista: id_vehiculo|estado|bateria */
#define CMD_VEH_ESTACION "VEH_ESTACION"

/* LISTAR_VEH
   Respuesta lista: id_vehiculo|estado|bateria|abrev_estacion */
#define CMD_LISTAR_VEH   "LISTAR_VEH"

/* RESERVAR id_vehiculo
   Respuesta: OK id_reserva   o   ERROR mensaje */
#define CMD_RESERVAR     "RESERVAR"

/* USAR_VEH id_vehiculo
   Inicia un trayecto. Respuesta: OK id_trayecto   o   ERROR */
#define CMD_USAR_VEH     "USAR_VEH"

/* FIN_TRAYECTO id_trayecto distancia_km
   Finaliza el trayecto activo. Respuesta: OK   o   ERROR */
#define CMD_FIN_TRAYECTO "FIN_TRAYECTO"

/* REPORTAR_AV id_vehiculo tipo descripcion
   Tipos validos: mecanica bateria carga otro
   Respuesta: OK   o   ERROR */
#define CMD_REPORTAR_AV  "REPORTAR_AV"

/* HISTORIAL
   Respuesta lista: id_trayecto|id_vehiculo|inicio|fin|distancia */
#define CMD_HISTORIAL    "HISTORIAL"

/* SALIR  – cierra la sesion */
#define CMD_SALIR        "SALIR"

/* --- Respuestas del servidor --- */
#define RESP_OK    "OK"
#define RESP_ERROR "ERROR"
#define RESP_FIN   "FIN"   /* Marca el final de una lista */

#endif /* PROTOCOLO_H */
