

#include <stdint.h>
#include "LPC17xx.h"

#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_uart.h"


/* =========================================================
   DEFINICIONES
   ========================================================= */


/* Efectos */
#define EFECTO_NORMAL           0
#define EFECTO_DISTORTION       1
#define EFECTO_FUZZ             2
#define EFECTO_BITCRUSH         3
#define EFECTO_DELAY            4
#define EFECTO_WHAMMY           5
#define CANTIDAD_EFECTOS        6

/* Botón */
#define PIN_BOTON               (1UL << 10)
#define RED_LED					22
#define GREEN_LED				25
#define	BLUE_LED				26


/* =========================================================
   VARIABLES GLOBALES
   ========================================================= */
volatile uint16_t muestra = 512;
/* Efecto seleccionado */
volatile uint8_t EFECTO = EFECTO_NORMAL;

/* Parámetros modificables por UART */
volatile uint16_t INTENSIDAD = 50;
volatile uint16_t DELAY = 200;
volatile uint16_t ATENUACION = 50;
volatile uint8_t respuestaUART = 0;
volatile uint8_t comandoInvalidoUART = 0;

#define TAM_COMANDO 10
char comando[TAM_COMANDO];
uint8_t indiceUART = 0;
/* Estado del botón */
volatile uint8_t botonPendiente = 0;
volatile uint8_t LED = 0;

/* Buffer  del Delay */
#define TAM_DELAY 10000
  // 100 ms a 40 kHz
volatile uint16_t RETARDO = 4000;
volatile uint16_t ECO1 = 50;
volatile uint16_t ECO2 = 35;
/* Bits de crush */
volatile uint8_t CRUSH = 3;

uint16_t bufferDelay[TAM_DELAY];
uint32_t indiceDelay = 0;

/* Buffer variables whammy */
#define TAM_WHAMMY 1024
#define VENTANA 512
#define RETARDO_MIN 32

uint16_t bufferWhammy[TAM_WHAMMY] = {0};
uint32_t indiceWhammy = 0;
uint32_t faseWhammy = 0;


/* UART: recepción de comandos */
volatile uint8_t comandoUART = 0;
volatile uint32_t valorUART = 0;
volatile uint8_t hayDigitosUART = 0;


/* =========================================================
   PROTOTIPOS
   ========================================================= */

/* Inicialización */
void InicializarADC(void);
void InicializarDAC(void);
void InicializarTimer(void);
void InicializarDMA(void);
void InicializarUART(void);
void InicializarBoton(void);
void InicializarBuffers(void);



/* =========================================================
   INICIALIZACIÓN ADC
   ========================================================= */

void InicializarADC(void)
{
    /*
       ADC PCLK = CCLK = 100 MHz.

       PCLKSEL0 bits 24:25:
       01 = CCLK.
    */
    ADC_Init(200000);
    LPC_SC->PCLKSEL0 &= ~(3UL << 24);
    LPC_SC->PCLKSEL0 |=  (1UL << 24);


    /* P0.23 = AD0.0 */
    ADC_PinConfig(ADC_CHANNEL_0);

    ADC_ChannelEnable(ADC_CHANNEL_0);

    ADC_EdgeStartConfig(ADC_START_ON_RISING);

    /* Disparo por MAT0.1 */
    ADC_StartCmd(ADC_START_ON_MAT01);
    /* Interrumpe en fin de conversion */
    NVIC_EnableIRQ(ADC_IRQn);
    ADC_IntEnable(ADC_INT_CH0);
}


/* =========================================================
   INICIALIZACIÓN DAC
   ========================================================= */

void InicializarDAC(void)
{

    DAC_Init();
    DAC_UpdateValue(512);
}


/* =========================================================
   INICIALIZACIÓN TIMER0
   ========================================================= */

void InicializarTimer(void)
{

    LPC_SC->PCLKSEL0 &= ~(3UL << 2);
    LPC_SC->PCLKSEL0 |=  (1UL << 2);

    TIM_TIMERCFG_T configuracionTimer;
    TIM_MATCHCFG_T configuracionMatch;

    configuracionTimer.prescaleOpt = TIM_TICK;
    configuracionTimer.prescaleValue = 1;

    TIM_InitTimer(LPC_TIM0, &configuracionTimer);

    /*
       Timer0 PCLK = CCLK = 100 MHz.

       PCLKSEL0 bits 2:3.
    */


    /*
       Match 1:

       100 MHz / 1250 = 80 kHz.

       MAT0.1 conmuta a 80 kHz.
       El flanco ascendente aparece a 40 kHz.
    */

    configuracionMatch.channel = TIM_MATCH_1;

    configuracionMatch.intEn = DISABLE;
    configuracionMatch.stopEn = DISABLE;
    configuracionMatch.resetEn = ENABLE;

    configuracionMatch.extOpt = TIM_TOGGLE;

    configuracionMatch.matchValue = 1250;

    TIM_ConfigMatch(LPC_TIM0, &configuracionMatch);

    TIM_ResetCounter(LPC_TIM0);
}


/* =========================================================
   ADC HANDER  (PROCESAMIENTO PRINCIPAL)
   ========================================================= */

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





/* =========================================================
   INICIALIZACIÓN DMA
   ========================================================= */



/* =========================================================
   INTERRUPCIÓN DMA
   ========================================================= */


/* =========================================================
   INICIALIZACIÓN UART0
   ========================================================= */


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



/* =========================================================
   TRANSMISIÓN UART
   ========================================================= */

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




	/* Leer UART sin bloquear */
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



/*
   Enviar nombre del efecto seleccionado.
*/



/* =========================================================
   PROCESAMIENTO DE COMANDOS UART
   ========================================================= */
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






/* =========================================================
   INTERRUPCIÓN UART0
   ========================================================= */


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



/* =========================================================
   INICIALIZACIÓN DEL BOTÓN
   ========================================================= */

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


/*
   Cambiar efecto circularmente.
*/

void CambiarEfecto(void)
{
    EFECTO++;
    if (EFECTO >= CANTIDAD_EFECTOS)
        EFECTO = EFECTO_NORMAL;
}


/* =========================================================
   INTERRUPCIÓN EXTERNA EINT0
   ========================================================= */

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

/* =========================================================
   ANTIRREBOTE
   ========================================================= */

void RetardoAntirrebote(void)
{
    volatile uint32_t i;

    for (i = 0; i < 50000; i++)
    {

        __NOP();
    }


}


/* =========================================================
   MAIN
   ========================================================= */

int main(void)
{



    InicializarBoton();

    InicializarADC();

    InicializarDAC();

    InicializarTimer();

    InicializarUART();

    for(uint32_t i = 0; i < TAM_DELAY; i++)
    {
        bufferDelay[i] = 512;
    }

    enviarUART("BIENVENIDO A EFFECMASTER9600 \r\n");
    enviarUART("SIN EFECTOS \r\n");

    enviarUART("\r\nCONTROL DE EFECTOS\r\n");
    enviarUART("I = Intensidad (0-100)\r\n");
    enviarUART("D = Delay (0-200)\r\n");
    enviarUART("A = Atenuacion (0-100)\r\n");
    enviarUART("S = Mostrar estado\r\n");

     TIM_Enable(LPC_TIM0);
     LPC_GPIO0->FIOCLR = (1<<RED_LED);
     LPC_GPIO3->FIOSET = (1<<GREEN_LED);
     LPC_GPIO3->FIOSET = (1<<BLUE_LED);

    while (1)
    {
        /*
           Gestión del botón.

           La interrupción cambia el efecto.
           El bucle principal transmite el nombre.
        */

        if (botonPendiente)
        {

        	botonPendiente = 0;

            RetardoAntirrebote();
            LPC_SC->EXTINT = 1UL;


            NVIC_ClearPendingIRQ(EINT0_IRQn);
            NVIC_EnableIRQ(EINT0_IRQn);
        }

        leerUART();

    }


}


