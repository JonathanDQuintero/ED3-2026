/*
* En esta archivo se desarrollar el codigo para el control de una semaforo, utilizado el led RGB
* que trae incorporado la LPC1769, donde se controlara el tiempo y los colores por medio del systick.
* El color rojo estara encendido por 5 segundos, color verde por 4 segundos y el azul por 2 segundos
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
    static uint32_t i=1100; // static por que quiero que conserve su valor entre interrupciones, que lo recuerde, y que no se reinicie cada vez que se ejecute la interrupcion
    i--;
    //la interrupcion cada en 11 segundos, donde el led rojo estara encendido por 5 seg, el azul por 2 seg
    //y el verde por 4 seg
    if(i==0){
        LPC_GPIO3->FIOSET= 1<<GREEN_LED;//Para que cuando se repita el ciclo el led verde no quede encendido y se apague
        i=1100;
    }else if(i>600){
        LPC_GPIO0->FIOCLR= 1<<RED_LED;//enciende el solo el led rojo, ya que lo demas inician apagados
    }else if(i>400){
        LPC_GPIO0->FIOSET= 1<<RED_LED;
        LPC_GPIO3->FIOCLR= 1<<BLUE_LED;//enciende el led azul y apaga el rojo
    }else if(i>0){
        LPC_GPIO3->FIOSET= 1<<BLUE_LED;
        LPC_GPIO3->FIOCLR= 1<<GREEN_LED;//enciende el led verde y apaga el azul
    }
} 
/*void SysTick_Handler(void)
{
    i--;

    if(i == 600)
    {
        // Pasaron 5 segundos
        LPC_GPIO0->FIOSET = 1 << RED_LED;   // apaga rojo
        LPC_GPIO3->FIOCLR = 1 << BLUE_LED;  // prende azul
    }
    else if(i == 400)
    {
        // Pasaron 2 segundos más
        LPC_GPIO3->FIOSET = 1 << BLUE_LED;   // apaga azul
        LPC_GPIO3->FIOCLR = 1 << GREEN_LED;  // prende verde
    }
    else if(i == 0)
    {
        // Pasaron 4 segundos más
        LPC_GPIO3->FIOSET = 1 << GREEN_LED; // apaga verde
        LPC_GPIO0->FIOCLR = 1 << RED_LED;   // prende rojo

        i = 1100;
    }
}
ESTA FORMA ESTARIA BIEN PERO QUE TENGO QUE HACER QUE EL LED ROJO INICIO ENCENDIDO DIRECTAMENTE EN confPCB    
    */