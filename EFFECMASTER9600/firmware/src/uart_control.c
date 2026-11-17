#include <stdint.h>
#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_uart.h"
#include "uart_control.h"
#include "uart_control.h"

void InicializarUART(void){

	    UART_CFG_T cfg;

	    cfg.baudRate = 9600;
	    cfg.dataBits = UART_DBITS_8;
	    cfg.parity = UART_PARITY_NONE;
	    cfg.stopBits = UART_STOPBIT_1;

	    UART_PinConfig(UART_TX0_P0_2);
	    UART_PinConfig(UART_RX0_P0_3);

	    UART_Init(UART0, &cfg);

	    UART_FIFO_CFG_T fifo;

	    fifo.resetRxBuf = ENABLE;
	    fifo.resetTxBuf = ENABLE;
	    fifo.dmaMode = DISABLE;
	    fifo.level = UART_FIFO_TRGLEV0;

	    UART_FIFOConfig(UART0, &fifo);
	    UART_TxEnable(UART0);

	    // Habilitar interrupcion por recepcion
	    UART_IntConfig(UART0, UART_INT_RBR, ENABLE);

	    NVIC_SetPriority(UART0_IRQn, 0);

	    NVIC_ClearPendingIRQ(UART0_IRQn);
	    NVIC_EnableIRQ(UART0_IRQn);
	}


void enviarUART(const char *mensaje){

	    while(*mensaje){
	        while(!(UART_GetLineStatus(UART0) & UART_LSR_THRE));

	        UART_SendByte(UART0, (uint8_t)*mensaje);
	        mensaje++;
	    }
	}


void enviarNumero(uint16_t numero){

	    char texto[6];
	    uint8_t i = 0;
	    uint8_t j;

	    do{
	        texto[i++] = (numero % 10) + '0';
	        numero /= 10;
	    }while(numero > 0);

	    for(j = i; j > 0; j--){

	        while(!(UART_GetLineStatus(UART0) & UART_LSR_THRE));

	        UART_SendByte(UART0, texto[j-1]);
	    }
	}


void mostrarEstado(void){

	    enviarUART("\r\nINTENSIDAD = ");
	    enviarNumero(INTENSIDAD);

	    enviarUART("\r\nDELAY = ");
	    enviarNumero(DELAY);

	    enviarUART("\r\nATENUACION = ");
	    enviarNumero(ATENUACION);

	    enviarUART("\r\n");
	}


void leerUART(void){

	    uint8_t dato;

	    while(UART_GetLineStatus(UART0) & UART_LSR_RDR){

	        dato = UART_ReceiveByte(UART0);

	        if(dato == '\r' || dato == '\n'){

	            if(indiceUART > 0){

	                comando[indiceUART] = '\0';

	                procesarComando();

	                indiceUART = 0;
	            }

	        }else{

	            if(indiceUART < TAM_COMANDO - 1){

	                comando[indiceUART++] = dato;

	            }else{

	                indiceUART = 0;
	                enviarUART("\r\nComando demasiado largo\r\n");
	            }
	        }
	    }
	}


void procesarComando(void){

	    uint16_t valor = 0;
	    uint8_t i = 1;

	    // Comando para mostrar estado
	    if(comando[0] == 'S' && comando[1] == '\0'){
	        mostrarEstado();
	        return;
	    }

	    // Comprobar que haya al menos un numero
	    if(comando[1] == '\0'){
	        enviarUART("\r\nComando invalido\r\n");
	        return;
	    }

	    // Convertir caracteres ASCII a numero
	    while(comando[i] != '\0'){

	        if(comando[i] < '0' || comando[i] > '9'){
	            enviarUART("\r\nNumero invalido\r\n");
	            return;
	        }

	        valor = valor * 10 + (comando[i] - '0');

	        // Rechazar valores fuera del rango maximo
	        if(valor > 200){
	            enviarUART("\r\nValor fuera de rango\r\n");
	            return;
	        }

	        i++;
	    }

	    switch(comando[0]){

	        case 'I':

	            if(valor <= 100){
	                INTENSIDAD = valor;
	                enviarUART("\r\nINTENSIDAD = ");
	                enviarNumero(INTENSIDAD);
	                enviarUART("\r\n");
	            }else{
	                enviarUART("\r\nINTENSIDAD: rango 0-100\r\n");
	            }

	            break;

	        case 'D':

	            DELAY = valor;

	            enviarUART("\r\nDELAY = ");
	            enviarNumero(DELAY);
	            enviarUART("\r\n");

	            break;

	        case 'A':

	            if(valor <= 100){
	                ATENUACION = valor;
	                enviarUART("\r\nATENUACION = ");
	                enviarNumero(ATENUACION);
	                enviarUART("\r\n");
	            }else{
	                enviarUART("\r\nATENUACION: rango 0-100\r\n");
	            }

	            break;

	        default:

	            enviarUART("\r\nComando desconocido\r\n");
	            break;
	    }
	}


void UART0_IRQHandler(void){

	    uint8_t dato;

	    // Leer todos los caracteres disponibles
	    while(UART_GetLineStatus(UART0) & UART_LSR_RDR){

	        dato = UART_ReceiveByte(UART0);

	        if(dato == '\r' || dato == '\n'){

	            if(indiceUART > 0){

	                comando[indiceUART] = '\0';

	                procesarComando();

	                indiceUART = 0;
	            }

	        }else{

	            if(indiceUART < TAM_COMANDO - 1){

	                comando[indiceUART++] = dato;

	            }else{

	                indiceUART = 0;
	                comandoInvalidoUART = 1;
	                respuestaUART = 'E';
	            }
	        }
	    }


	    RETARDO = DELAY*40;

	    if(INTENSIDAD<=20){
	       CRUSH = 1;
	    }
	    else if (INTENSIDAD<20&&INTENSIDAD<=40){
		   CRUSH = 2;
	    }
	    else if (INTENSIDAD<40&&INTENSIDAD<=60){
		   CRUSH = 3;
	    }
	    else if (INTENSIDAD<60&&INTENSIDAD<=70){
		   CRUSH = 4;
	    }
	    else if (INTENSIDAD<70&&INTENSIDAD<=80){
		   CRUSH = 5;
	    } else{
	       CRUSH = 6;
	    }

	    ECO1= 100-ATENUACION;
	    if(ECO1<=15){
	     ECO2 = 0;
	    }else{
	    	ECO2 = ECO1-15;
	    }





	}


void MostrarBienvenida(void)
{
    enviarUART("BIENVENIDO A EFFECMASTER9600 \r\n");
    enviarUART("SIN EFECTOS \r\n");

    enviarUART("\r\nCONTROL DE EFECTOS\r\n");
    enviarUART("I = Intensidad (0-100)\r\n");
    enviarUART("D = Delay (0-200)\r\n");
    enviarUART("A = Atenuacion (0-100)\r\n");
    enviarUART("S = Mostrar estado\r\n");
}
