#include "LPC17xx.h"

static volatile uint32_t captura_anterior = 0;
static volatile uint32_t periodo = 0;
static volatile float temperatura = 0;

void confCAP(void);
void confTimer0(void);

int main(void){

    confCAP();
    confTimer0();

    while(1){
        // La temperatura queda disponible
        // en la variable "temperatura"
    }
}


void confCAP(void){

    /*
     * Configurar acá mediante PINSEL
     * el pin correspondiente como CAP0.0.
     *
     * La selección exacta depende del pin CAP0.0
     * que estés usando en tu placa.
     */
}


void confTimer0(void){

    // Encender Timer0
    LPC_SC->PCONP |= (1 << 1);

    // PCLK_TIMER0 = CCLK
    LPC_SC->PCLKSEL0 &= ~(0b11 << 2);
    LPC_SC->PCLKSEL0 |=  (0b01 << 2);

    // Timer mode
    LPC_TIM0->CTCR &= ~(0b11 << 0);

    // Si PCLK = 100 MHz:
    // TC incrementa cada 1 us
    LPC_TIM0->PR = 99;

    /*
     * CCR = Capture Control Register
     *
     * Para CAP0.0:
     * bit 0 = captura en flanco de subida
     * bit 1 = captura en flanco de bajada
     * bit 2 = genera interrupción al capturar
     */

    LPC_TIM0->CCR |= (1 << 0);   // Rising edge
    LPC_TIM0->CCR &= ~(1 << 1);  // No falling edge
    LPC_TIM0->CCR |= (1 << 2);   // Interrupción

    // Reset inicial
    LPC_TIM0->TCR |=  (1 << 1);
    LPC_TIM0->TCR &= ~(1 << 1);

    // Habilitar interrupción
    NVIC_EnableIRQ(TIMER0_IRQn);

    // Arrancar Timer
    LPC_TIM0->TCR |= (1 << 0);
}


void TIMER0_IRQHandler(void){

    static uint8_t primera_captura = 1;
    uint32_t captura_actual;

    // IR bit 4 -> interrupción CAP0.0
    if(LPC_TIM0->IR & (1 << 4)){

        // El hardware guardó TC automáticamente en CR0
        captura_actual = LPC_TIM0->CR0;

        if(primera_captura){

            captura_anterior = captura_actual;
            primera_captura = 0;

        }else{

            periodo = captura_actual - captura_anterior;
            captura_anterior = captura_actual;

            /*
             * TC incrementa cada 1 us.
             *
             * periodo está expresado en us.
             *
             * f = 1/T
             *
             * Si T está en us:
             * f = 1.000.000 / periodo
             */

            float frecuencia = 1000000.0f / periodo;

            /*
             * Sensor hipotético:
             *
             * 10 Hz = 1 °C
             *
             * T = f / 10
             */

            temperatura = frecuencia / 10.0f;
        }

        // Limpiar bandera CAP0.0
        LPC_TIM0->IR = (1 << 4);
    }
}