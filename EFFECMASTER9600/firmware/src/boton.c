#include <stdint.h>
#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_uart.h"
#include "boton.h"
#include "uart_control.h"

void InicializarBoton(void)
{
	/*LED P0.22*/
	 /* Sin pull-up ni pull-down */
	    LPC_PINCON->PINMODE1 &= ~(3UL << 12);
	    LPC_PINCON->PINMODE1 |=  (2UL << 12);

	    /* Configurar como salida */
	    LPC_GPIO0->FIODIR |= (1UL << RED_LED);
	/*LED P3.25*/
	    LPC_PINCON->PINMODE7 &= ~(3UL << 18);
	    LPC_PINCON->PINMODE7 |=  (2UL << 18);

	    // Salida
	        LPC_GPIO3->FIODIR |= (1UL << GREEN_LED);
	 /*LED P3.26*/
	   // Sin pull-up / pull-down
	     LPC_PINCON->PINMODE7 &= ~(3UL << 20);
	     LPC_PINCON->PINMODE7 |=  (2UL << 20);
	      // Salida
	     LPC_GPIO3->FIODIR |= (1UL << BLUE_LED);


    /*
       P2.10 = EINT0.

       PINSEL4 bits 20:21:
       01 = EINT0.

       PINMODE4 bits 20:21:
       00 = pull-up interno.
    */

    LPC_PINCON->PINSEL4 &= ~(3UL << 20);
    LPC_PINCON->PINSEL4 |=  (1UL << 20);

    LPC_PINCON->PINMODE4 &= ~(3UL << 20);

    /*
       EINT0:
       interrupción sensible a flanco.
       Flanco descendente.
    */

    LPC_SC->EXTMODE |= 1UL;
    LPC_SC->EXTPOLAR &= ~1UL;

    LPC_SC->EXTINT = 1UL;
    NVIC_SetPriority(EINT0_IRQn, 0);
    NVIC_ClearPendingIRQ(EINT0_IRQn);
    NVIC_EnableIRQ(EINT0_IRQn);
}


void CambiarEfecto(void)
{
    EFECTO++;
    if (EFECTO >= CANTIDAD_EFECTOS)
        EFECTO = EFECTO_NORMAL;
}


void EINT0_IRQHandler(void)
{
        botonPendiente = 1;
        /* Cambiar estado del LED */
        LED++;
        if(LED>5){
          	LED=0;
          }

        switch(LED){
                case 0:
                LPC_GPIO0->FIOCLR = (1<<RED_LED);

            	LPC_GPIO3->FIOSET = (1<<GREEN_LED);
            	LPC_GPIO3->FIOSET = (1<<BLUE_LED);
                break;
                case 1:
                LPC_GPIO3->FIOCLR = (1<<GREEN_LED);
                LPC_GPIO0->FIOCLR = (1<<RED_LED);

                LPC_GPIO3->FIOSET = (1<<BLUE_LED);
                break;
                case 2:
                LPC_GPIO3->FIOCLR = (1<<GREEN_LED);
                LPC_GPIO3->FIOCLR = (1<<BLUE_LED);

            	LPC_GPIO0->FIOSET = (1<<RED_LED);
                break;
                case 3:
                LPC_GPIO0->FIOCLR = (1<<RED_LED);
                LPC_GPIO3->FIOCLR = (1<<BLUE_LED);

            	LPC_GPIO3->FIOSET = (1<<GREEN_LED);
                break;
                case 4:
                LPC_GPIO3->FIOCLR = (1<<BLUE_LED);

            	LPC_GPIO3->FIOSET = (1<<GREEN_LED);
            	LPC_GPIO0->FIOSET = (1<<RED_LED);
            	break;
                case 5:
                LPC_GPIO3->FIOSET = (1<<BLUE_LED);

            	LPC_GPIO3->FIOCLR = (1<<GREEN_LED);
            	LPC_GPIO0->FIOCLR = (1<<RED_LED);
                break;
                	}




        /* Cambiar efecto */
        CambiarEfecto();
        switch(EFECTO){
        case 0:
        enviarUART("SIN EFECTOS \r\n");
        break;

        case 1:
        enviarUART("DISTORSION \r\n");

        break;

        case 2:
        enviarUART("FUZZ \r\n");
        break;

        case 3:
        enviarUART("BIT CRUNCH \r\n");
        break;

        case 4:
        enviarUART("DELAY \r\n");
        break;
        case 5:
        enviarUART("WHAMMY \r\n");
        break;
        }

        /* Limpiar bandera EINT0 */
        LPC_SC->EXTINT = (1UL << 0);


}


void RetardoAntirrebote(void)
{
    volatile uint32_t i;

    for (i = 0; i < 50000; i++)
    {

        __NOP();
    }


}


void AtenderBoton(void)
{
        if (botonPendiente)
        {

        	botonPendiente = 0;

            RetardoAntirrebote();
            LPC_SC->EXTINT = 1UL;


            NVIC_ClearPendingIRQ(EINT0_IRQn);
            NVIC_EnableIRQ(EINT0_IRQn);
        }
}
