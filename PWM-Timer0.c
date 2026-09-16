#include "LPC17xx.h"

#define RED_LED     22
#define GREEN_LED   25
#define BLUE_LED    26
#define BUTTON      20 

static volatile uint8_t ESTADO=0;

void confgGPIO(void);
void confgTMR0(void);
void confgENT0(void);

int main(void){
    confgGPIO();
    confgTMR0();
    confgEINT0();
    while(1){

    }
}
void confgGPIO(void){
    //led rojo P0.22
    LPC_PINCON->PINSEL1 &=~(0b11<<12);

    LPC_PINCON->PINMODE1 &=~(0b11<<12);
    LPC_PINCON->PINMODE1 |=(0b10<<12);
    // LED VERDE P3.25
    LPC_PINCON->PINSEL7 &=~(0b11<<18);

    LPC_PINCON->PINMODE7 &= ~(0b11<<18);
    LPC_PINCON->PINMODE7 |= (0b10<<18);
    // LED AZUL P3.26
    LPC_PINCON->PINSEL7 &=~(0b11<<20);

    LPC_PINCON->PINMODE7 &= ~(0b11<<20);
    LPC_PINCON->PINMODE7 |= (0b10<<20);
    //BOTON P2.10
    LPC_PINCON->PINSEL4 &=~(0b11<<BUTTON);
    LPC_PINCON->PINSEL4 |= (0b10<<BUTTON);//Configuro como funcion EINT0
    //Configuracion de los pines como salida
    LPC_GPIO0->FIODIR |= (0b01<<RED_LED);
    LPC_GPIO0->FIOSET = (0b01<<RED_LED); //Como es salida y es activo por bajo, lo ponemos en 1 para que comience apagado

    LPC_GPIO3->FIODIR |= (0b01<<GREEN_LED);
    LPC_GPIO3->FIOSET =  (0b01<<GREEN_LED);

    LPC_GPIO3->FIODIR |= (0b01<<BLUE_LED);
    LPC_GPIO3->FIOSET = (0b01<<BLUE_LED);
}
void confgEINT0(){
    LPC_SC->EXTMODE |=(1<<0);//Activado por flanco
    LPC_SC->EXTPOLAR &=~(1<<0);//Activo por bajo
    LPC_SC->EXTINT |= (1<<0);//Se limpia la bander de interrupcion
    NVIC_EnableIRQ(EINT0_IRQn);
}
void EINT0_IRQHandler(void){
    if(LPC_SC->EXTINT & (1<<0)){
        ESTADO++;
    if(ESTADO>2){
        ESTADO=0;
    }
    LPC_SC->EXTINT |= (1<<0);//LIMPIO LA BANDERA
    }
}
void confgTMR0(void){
    //Configuracion en clocking and power control
    LPC_SC->PCONP |= (1<<1);//Se enciende el periferico del timer0 
    LPC_SC->PCLKSEL0 |=(1<<2);//Se configura el perifercio PCLK con el la frecuencia interna del CLK interno
    //TMR0
    LPC_TIM0->CTCR &=~(3<<0);//Modo TIMER y sin COUNTER
    LPC_TIM0->PR=99;//Se configura el prescaler para que cuente cada 100 ciclos de clock(osea cada 1us)
    LPC_TIM0->MR0=100;//Se configura el match register para que genere una interrupcion cada 100us((PR+1)*MR0=100us)
    LPC_TIM0->MCR|=(0b11<<0);//Se configura el match control register para que genere una interrupcion y resetee el timer cuando llegue a MR0

    LPC_TIM0->TCR |=(1<<0);//se habilita el TC y PC para que comienze a contar
    NVIC_EnableIRQ(TIMER0_IRQn);

}
void TIMER0_IRQHandler (void){
    static uint8_t  PWM=0;
    static uint8_t  DUTY=60;

    if(LPC_TIM0->IR & (1<<0)){ //verificacion si hay interrupcion por MR0
        if(PWM<DUTY){
            switch (ESTADO){
            case 0:
            LPC_GPIO3->FIOSET = (1<<GREEN_LED);
            LPC_GPIO0->FIOCLR = (1<<RED_LED);
            break;
            case 1:
            LPC_GPIO0->FIOSET = (1<<RED_LED);
            LPC_GPIO3->FIOCLR = (1<<BLUE_LED);
            break;
            case 2:
            LPC_GPIO3->FIOSET = (1<<BLUE_LED);
            LPC_GPIO3->FIOCLR = (1<<GREEN_LED);
            break;
        }
    }else{
            LPC_GPIO0->FIOSET = (1<<RED_LED);
            LPC_GPIO3->FIOSET = (1<<BLUE_LED);
            LPC_GPIO3->FIOSET = (1<<GREEN_LED);
        }
    PWM++;
    if(PWM==100){
        PWM=0
    }
    LPC_TIM0->IR = (1<<0);//Limpio la bandera con 1 por que es un registro de escritura 1 para limpiar la bandera(W1C)
    }
}