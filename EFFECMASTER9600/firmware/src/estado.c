#include "estado.h"

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


