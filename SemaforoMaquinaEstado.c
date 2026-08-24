/*
* En esta archivo se desarrollar el codigo para el control de una semaforo pero utilzando maquina de estados
*utilizando el led RGB que trae incorporado la LPC1769, donde se controlara el tiempo y los colores 
*por medio del systick.El color rojo estara encendido por 5 segundos, color verde por 4 segundos y el azul
*por 2 segundos
* 
*/

#include "LPC17xx.h"

#define RED_LED     22
#define GREEN_LED   25
#define BLUE_LED    26


void confPCB(void);
void confSystick(void);

int main(void){

    confPCB();
    confSystick();
    while(1){
        //todo lo hace el Handler del Systick
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

    //Configuracion de los pines como salida
    LPC_GPIO0->FIODIR |=1 <<RED_LED;
    LPC_GPIO0->FIOSET = 1 <<RED_LED; //Como es salida y es activo por bajo, lo ponemos en 1 para que comience apagado

    LPC_GPIO3->FIODIR |=1 <<GREEN_LED;
    LPC_GPIO3->FIOSET = 1 <<GREEN_LED;

    LPC_GPIO3->FIODIR |=1 <<BLUE_LED;
    LPC_GPIO3->FIOSET = 1 <<BLUE_LED;
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
    static EstadoSemaforo estado=ROJO;
    static uint32_t contador=0;

    contador++;
    switch(estado){
        case ROJO:
            LPC_GPIO0->FIOCLR= 1<<RED_LED;
            if(contador>=500){
                LPC_GPIO0->FIOSET= 1<<RED_LED;
                estado=AZUL;
                contador=0;
            }
            break;
        case AZUL:
            LPC_GPIO3->FIOCLR= 1<<BLUE_LED;
            if(contador>=200){
                LPC_GPIO3->FIOSET= 1<<BLUE_LED;
                estado=VERDE;
                contador=0;
            }
            break;
        case VERDE:
            LPC_GPIO3->FIOCLR= 1<<GREEN_LED;
            if(contador>=400){
                LPC_GPIO3->FIOSET= 1<<GREEN_LED;
                estado=ROJO;
                contador=0;
            }   
            break; 
    }
} 
