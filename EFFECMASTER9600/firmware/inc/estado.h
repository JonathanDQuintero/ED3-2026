#ifndef EFFECMASTER_ESTADO_H
#define EFFECMASTER_ESTADO_H
#include <stdint.h>
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


extern volatile uint16_t muestra;
/* Efecto seleccionado */
extern volatile uint8_t EFECTO;

/* Parámetros modificables por UART */
extern volatile uint16_t INTENSIDAD;
extern volatile uint16_t DELAY;
extern volatile uint16_t ATENUACION;
extern volatile uint8_t respuestaUART;
extern volatile uint8_t comandoInvalidoUART;

#define TAM_COMANDO 10
extern char comando[TAM_COMANDO];
extern uint8_t indiceUART;
/* Estado del botón */
extern volatile uint8_t botonPendiente;
extern volatile uint8_t LED;

/* Buffer  del Delay */
#define TAM_DELAY 10000
  // 100 ms a 40 kHz
extern volatile uint16_t RETARDO;
extern volatile uint16_t ECO1;
extern volatile uint16_t ECO2;
/* Bits de crush */
extern volatile uint8_t CRUSH;

extern uint16_t bufferDelay[TAM_DELAY];
extern uint32_t indiceDelay;

/* Buffer variables whammy */
#define TAM_WHAMMY 1024
#define VENTANA 512
#define RETARDO_MIN 32

extern uint16_t bufferWhammy[TAM_WHAMMY];
extern uint32_t indiceWhammy;
extern uint32_t faseWhammy;


/* UART: recepción de comandos */
extern volatile uint8_t comandoUART;
extern volatile uint32_t valorUART;
extern volatile uint8_t hayDigitosUART;


#endif
