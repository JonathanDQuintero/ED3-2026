#include <stdint.h>
#include "LPC17xx.h"
#include "lpc17xx_timer.h"
#include "estado.h"
#include "perifericos.h"
#include "audio.h"
#include "uart_control.h"
#include "boton.h"

int main(void)
{
    InicializarBoton();
    InicializarADC();
    InicializarDAC();
    InicializarTimer();
    InicializarUART();

    InicializarBuffers();
    MostrarBienvenida();

    TIM_Enable(LPC_TIM0);
    LPC_GPIO0->FIOCLR = (1<<RED_LED);
    LPC_GPIO3->FIOSET = (1<<GREEN_LED);
    LPC_GPIO3->FIOSET = (1<<BLUE_LED);

    while (1)
    {
        AtenderBoton();
        leerUART();
    }
}
