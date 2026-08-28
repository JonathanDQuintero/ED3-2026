/*
* En esta archivo se desarrollar el codigo para el control de la luces RGB haciendo que esten parpadeando
y que con un switch puedas intercambiar el periodo de parpadeo de 10ms a 100ms y viceversa, dependiendo en
que posicion este el switch. Ademas se utiliza un boton para cambiar el color de la luz del led RGB.
se utiliza PO.8 como entrada para el switch utilizadno interrupcion de GPIO y el boton con interrupcion 
externa EINT1 donde se utilizar el P2.10 como entrada para el boton.
* 
*/

#include "LPC17xx.h"

#define RED_LED     22
#define GREEN_LED   25
#define BLUE_LED    26
#define SWITCH       8
#define BUTTON      10

static volatile uint8_t ESTADO=0; //variable para cambiar el color del led RGB
void confPCB(void);
void confEINT0(void);
void confSystick(void);

int main(void){

    confPCB();
    confEINT0();
    confSystick();
    while(1){
        //todo lo hace los handlers
    }

}

void confPCB(void){
    //configuracion de los pines como GPIO
    // LED ROJO P0.22
    LPC_PINCON->PINSEL1 &= ~(3<<12);

    LPC_PINCON->PINMODE1 &= ~(3<<12);
    LPC_PINCON->PINMODE1 |= (2<<12); //sin pull up/pull down por que es una salida
    // LED VERDE P3.25
    LPC_PINCON->PINSEL7 &=~(3<<18);

    LPC_PINCON->PINMODE7 &= ~(3<<18);
    LPC_PINCON->PINMODE7 |= (2<<18);
    // LED AZUL P3.26
    LPC_PINCON->PINSEL7 &=~(3<<20);

    LPC_PINCON->PINMODE7 &= ~(3<<20);
    LPC_PINCON->PINMODE7 |= (2<<20);
    // Switch P0.8
    LPC_PINCON->PINSEL0 &= ~(3<<16);
    LPC_PINCON->PINMODE0 &= ~(3<<16);// pull up

    //Configuracion de los pines como salida
    LPC_GPIO0->FIODIR |=1 <<RED_LED;
    LPC_GPIO0->FIOSET = 1 <<RED_LED; //Como es salida y es activo por bajo, lo ponemos en 1 para que comience apagado

    LPC_GPIO3->FIODIR |=1 <<GREEN_LED;
    LPC_GPIO3->FIOSET = 1 <<GREEN_LED;

    LPC_GPIO3->FIODIR |=1 <<BLUE_LED;
    LPC_GPIO3->FIOSET = 1 <<BLUE_LED;
    
    //ENTRADAS
    //configuracion del switch como entrada
    LPC_GPIO0->FIODIR &= ~(1<<SWITCH);
    LPC_GPIOINT->IO0IntEnF |= (1<<SWITCH); //interrupcion por flanco de bajada
    LPC_GPIOINT->IO0IntEnR |= (1<<SWITCH); //interrupcion por flanco de subida

    NVIC_EnableIRQ(EINT3_IRQn);
    
}
void EINT3_IRQHandler(void){
    if(LPC_GPIOINT->IO0IntStatR & (1<<SWITCH)){
        SysTick->LOAD = (SystemCoreClock/100)-1;//cambio a 10ms
        SysTick->VAL=0; // reinicia el contador
        LPC_GPIOINT->IO0IntClr |= (1<<SWITCH);
    }else if(LPC_GPIOINT->IO0IntStatF & (1<<SWITCH)){
        SysTick->LOAD = (SystemCoreClock/10)-1; // cambio a 100ms
        SysTick->VAL=0;
        LPC_GPIOINT->IO0IntClr |= (1<<SWITCH);

    }
}
void confEINT0(void){
    //configuracion del boton como interrupcion externa
    LPC_PINCON->PINSEL4 &= ~(3<<20);
    LPC_PINCON->PINSEL4 |= (1<<20);

    LPC_SC->EXTMODE |= (1<<0); //Interrupcion por flanco
    LPC_SC->EXTPOLAR &= ~(1<<0);
    LPC_SC->EXTINT |= (1<<0); //Limpiar la bander de interrupcion
    NVIC_EnableIRQ(EINT0_IRQn);
}
void EINT0_IRQHandler(void){
    if(LPC_SC->EXTINT & (1<<0)){
        //Cambia el color del led RGB
        ESTADO++;
        if(ESTADO>2){
            ESTADO=0;
        }
        LPC_SC->EXTINT |= (1<<0);//limpia la bandera de interrupcion
    }
}
void confSystick(void){
    //configuracion del Systick 
    //para que interrumpa cada 10ms
    SysTick->LOAD =(SystemCoreClock/100)-1;
    //limpia el valor del contador
    SysTick->VAL=0;
    //hablitacion del systick, del interrupcion y seleccion del reloj
    SysTick->CTRL= (1<<0)|
                    (1<<1)|
                    (1<<2)|;//SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

}
void SysTick_Handler(void){
    static uint32_t ENCENDIDO=0;
    ENCENDIDO=!ENCENDIDO;// Para que cambie el estado del led y parpadee
    if (ENCENDIDO){
        switch(ESTADO){
            case 0:
                LPC_GPIO3->FIOSET = 1<<BLUE_LED;
                LPC_GPIO0->FIOCLR = 1<<RED_LED;
                break;
            case 1:
                LPC_GPIO0->FIOSET = 1<<RED_LED;
                LPC_GPIO3->FIOCLR = 1<<GREEN_LED;
                break;
            case 2:
                LPC_GPIO3->FIOSET = 1<<GREEN_LED;
                LPC_GPIO3->FIOCLR = 1<<BLUE_LED;
                break;
    }else{
        switch(ESTADO){
            case 0:
                LPC_GPIO3->FIOSET = 1<<BLUE_LED;
                LPC_GPIO0->FIOSET = 1<<RED_LED;
                break;
            case 1:
                LPC_GPIO0->FIOSET = 1<<RED_LED;
                LPC_GPIO3->FIOSET = 1<<GREEN_LED;
                break;
            case 2:
                LPC_GPIO3->FIOSET = 1<<GREEN_LED;
                LPC_GPIO3->FIOSET = 1<<BLUE_LED;
                break;
        }
    }
    
} 
