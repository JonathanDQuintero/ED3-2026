#include <stdint.h>
#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_uart.h"
#include "audio.h"

void ADC_IRQHandler(){

	    muestra = ADC_ChannelGetData(ADC_CHANNEL_0);
	    muestra = (muestra>>2);
	    switch(EFECTO)
	    		{
	    			case EFECTO_NORMAL:	/*	No hay efecto*/

	    				DAC_UpdateValue(muestra);

	    				break;

	    			case EFECTO_DISTORTION: /*	Efecto distorsion*/
	    			{
	    			    uint32_t ganancia = 50 + INTENSIDAD*5;
	    			    uint32_t amplitud;
	    			    uint32_t salida;

	    			    if(muestra >= 512)
	    			    {
	    			        amplitud = (muestra - 512) * ganancia;

	    			        salida = 512 + (amplitud * 512) /
	    			                       (51200 + amplitud);
	    			    }
	    			    else
	    			    {
	    			        amplitud = (512 - muestra) * ganancia;

	    			        salida = 512 - (amplitud * 512) /
	    			                       (51200 + amplitud);
	    			    }

	    			    DAC_UpdateValue(salida);


	    			}
	    			    break;

	    			case EFECTO_FUZZ:                       /*Efecto fuzz*/

	    				if(muestra > 1023-4*INTENSIDAD)
	    					 muestra = 1023-4*INTENSIDAD   ;
	    				else if(muestra < 4*INTENSIDAD)
	    					 muestra = 4*INTENSIDAD		   ;
	    				DAC_UpdateValue(muestra)	       ;

	    				break;

	    			case EFECTO_BITCRUSH:                  /*	Efecto de BITCRUSH*/
                        muestra = (muestra>>CRUSH)&0x03FF;
	    				DAC_UpdateValue(muestra<<CRUSH);

	    				break;

	    			case EFECTO_DELAY:                     /*	Efecto de delay*/
	    			{
	    			    int32_t actual;
	    			    int32_t anterior;
	    			    int32_t salida;
	    			    int32_t guardar;
	    			    uint32_t lectura;

	    			    /* Obtener la posición de la muestra anterior */
	    			    lectura = (indiceDelay + TAM_DELAY - RETARDO) % TAM_DELAY;

	    			    /* Leer la muestra anterior */
	    			    anterior = (int32_t)bufferDelay[lectura] - 512;

	    			    /* Señal actual */
	    			    actual = (int32_t)muestra - 512;

	    			    /* Sumar señal actual +  eco */
	    			    salida = actual + (anterior * ECO1) / 100;

	    			    /* Guardar con realimentación */
	    			    guardar = actual + (anterior * ECO2) / 100;

	    			    /* Limitar al rango de 10 bits con signo centrado */
	    			    if(guardar > 511)
	    			        guardar = 511;

	    			    if(guardar < -512)
	    			        guardar = -512;

	    			    /* Guardar nueva muestra en el buffer */
	    			    bufferDelay[indiceDelay] = (uint16_t)(guardar + 512);

	    			    /* Avanzar índice circular */
	    			    indiceDelay++;

	    			    if(indiceDelay >= TAM_DELAY)
	    			        indiceDelay = 0;

	    			    /* Evitar saturación de salida */
	    			    if(salida > 511)
	    			        salida = 511;

	    			    if(salida < -512)
	    			        salida = -512;

	    			    /* Enviar al DAC */
	    			    DAC_UpdateValue((uint16_t)(salida + 512));
	    			}

	    				break;


	    			case EFECTO_WHAMMY:

	    			    uint32_t fase1, fase2;
	    			    uint32_t lectura1, lectura2;
	    			    uint32_t peso1, peso2;
	    			    uint32_t fraccion;
	    			    uint32_t paso;

	    			    int32_t a, b;
	    			    int32_t sonido1, sonido2;
	    			    int32_t salida;

	    			    // Guardar muestra actual en el buffer
	    			    bufferWhammy[indiceWhammy] = muestra;

	    			    // INTENSIDAD = 0: sonido original
	    			    if(INTENSIDAD < 20){

	    			        DAC_UpdateValue(muestra);

	    			    }else{

	    			        // Velocidad adicional de lectura
	    			        paso = ((uint32_t)INTENSIDAD * 65536u) / 100u;

	    			        // Dos lectores separados media ventana
	    			        fase1 = faseWhammy;
	    			        fase2 = (fase1 + (256u << 16))
	    			                % (VENTANA << 16);

	    			        // Primera posicion de lectura
	    			        lectura1 = (indiceWhammy + TAM_WHAMMY
	    			                   - RETARDO_MIN - VENTANA
	    			                   + (fase1 >> 16)) % TAM_WHAMMY;

	    			        // Interpolacion lineal
	    			        fraccion = (fase1 & 0xFFFFu) >> 8;

	    			        a = (int32_t)bufferWhammy[lectura1] - 512;
	    			        b = (int32_t)bufferWhammy[(lectura1 + 1)
	    			                                 % TAM_WHAMMY] - 512;

	    			        sonido1 = a + ((b - a) * (int32_t)fraccion) / 256;

	    			        // Segunda posicion de lectura
	    			        lectura2 = (indiceWhammy + TAM_WHAMMY
	    			                   - RETARDO_MIN - VENTANA
	    			                   + (fase2 >> 16)) % TAM_WHAMMY;

	    			        fraccion = (fase2 & 0xFFFFu) >> 8;

	    			        a = (int32_t)bufferWhammy[lectura2] - 512;
	    			        b = (int32_t)bufferWhammy[(lectura2 + 1)
	    			                                 % TAM_WHAMMY] - 512;

	    			        sonido2 = a + ((b - a) * (int32_t)fraccion) / 256;

	    			        // Pesos triangulares para mezclar los lectores
	    			        peso1 = fase1 >> 16;
	    			        if(peso1 > 256)
	    			            peso1 = 512 - peso1;

	    			        peso2 = 256 - peso1;

	    			        salida = (sonido1 * (int32_t)peso1
	    			                + sonido2 * (int32_t)peso2) / 256;

	    			        // Limitar a 10 bits con signo centrado
	    			        if(salida > 511)
	    			            salida = 511;

	    			        if(salida < -512)
	    			            salida = -512;

	    			        DAC_UpdateValue((uint16_t)(salida + 512));

	    			        // Avanzar la fase de lectura
	    			        faseWhammy += paso;

	    			        if(faseWhammy >= (VENTANA << 16))
	    			            faseWhammy -= (VENTANA << 16);
	    			    }

	    			    // Avanzar indice circular de escritura
	    			    indiceWhammy++;

	    			    if(indiceWhammy >= TAM_WHAMMY)
	    			        indiceWhammy = 0;

	    			    break;
	    			default:
	    			    break;
	    			}


	    		}


void InicializarBuffers(void)
{
    for(uint32_t i = 0; i < TAM_DELAY; i++)
    {
        bufferDelay[i] = 512;
    }
}
