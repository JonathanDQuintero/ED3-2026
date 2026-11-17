#ifndef EFFECMASTER_BOTON_H
#define EFFECMASTER_BOTON_H
#include "estado.h"

void InicializarBoton(void);
void CambiarEfecto(void);
void EINT0_IRQHandler(void);
void RetardoAntirrebote(void);
void AtenderBoton(void);

#endif
