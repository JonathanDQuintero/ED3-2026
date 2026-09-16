#include "LPC17xx.h"

#define MAT_PIN 28

void confgMAT(void);
void confgTMR0(void);

int main(void){

    confgMAT();
    confgTMR0();

    while(1){
        // La señal la genera el hardware
    }
}


void confgMAT(void){

    // P1.28 -> MAT0.0

    // Limpiamos los bits correspondientes a P1.28
    LPC_PINCON->PINSEL3 &= ~(0b11 << 24);

    // Seleccionamos la función MAT0.0
    LPC_PINCON->PINSEL3 |=  (0b11 << 24);
}


void confgTMR0(void){

    // Alimentar Timer0
    LPC_SC->PCONP |= (1 << 1);


    // PCLK_TIMER0 = CCLK
    // PCLKSEL0[3:2] = 01

    LPC_SC->PCLKSEL0 &= ~(0b11 << 2);
    LPC_SC->PCLKSEL0 |=  (0b01 << 2);


    // Timer mode
    LPC_TIM0->CTCR &= ~(0b11 << 0);


    // Prescaler
    // PCLK = 100 MHz
    // PR = 99
    // TC incrementa cada 1 us

    LPC_TIM0->PR = 99;


    // Match cada 500 us
    LPC_TIM0->MR0 = 500;


    // MCR
    // bit 0 = 0 -> NO interrupción
    // bit 1 = 1 -> reset TC cuando TC == MR0
    // bit 2 = 0 -> NO detener Timer

    LPC_TIM0->MCR &= ~(0b111 << 0);
    LPC_TIM0->MCR |=  (1 << 1);


    // EMR
    // EMC0 = bits 5:4
    //
    // 00 -> nada
    // 01 -> clear
    // 10 -> set
    // 11 -> toggle
    //
    // Queremos TOGGLE de MAT0.0

    LPC_TIM0->EMR &= ~(0b11 << 4);
    LPC_TIM0->EMR |=  (0b11 << 4);

    // Reset inicial de TC y PC

    LPC_TIM0->TCR |=  (1 << 1);
    LPC_TIM0->TCR &= ~(1 << 1);


    // Arrancar Timer0

    LPC_TIM0->TCR |= (1 << 0);
} 