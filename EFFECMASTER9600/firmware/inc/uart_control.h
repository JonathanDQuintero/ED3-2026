#ifndef EFFECMASTER_UART_CONTROL_H
#define EFFECMASTER_UART_CONTROL_H
#include "estado.h"

void InicializarUART(void);
void enviarUART(const char *mensaje);
void enviarNumero(uint16_t numero);
void mostrarEstado(void);
void leerUART(void);
void procesarComando(void);
void UART0_IRQHandler(void);
void MostrarBienvenida(void);

#endif
