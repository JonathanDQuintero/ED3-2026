#include "LPC17xx.h"

//Mascara macro para pin
#define PIN_MASK(x) (0x1<<(x))
//Mascara macro para puerto-pin
#define PORT_PIN_MASK(x,y) (((0x1<<(x))-1)<<(y))

#define RED_LED   22
#define GREEN_LED 25
#define BLUE_LED  26
#define BUTTON    10
//Pines 
#define RED_PIN     PIN_MASK(RED_LED)
#define GREEN_PIN   PIN_MASK(GREEN_LED)
#define BLUE_PIN    PIN_MASK(BLUE_LED)
#define BUTTON_PINSEL  PIN_MASK(BUTTON*2)
//Puerto-pin
#define RED_PORT_PIN    PORT_PIN_MASK(2,(RED_LED-16)*2)
#define GREEN_PORT_PIN  PORT_PIN_MASK(2,(GREEN_LED-16)*2)
#define BLUE_PORT_PIN   PORT_PIN_MASK(2,(BLUE_LED-16)*2)
#define BUTTON_PORT_PIN PORT_PIN_MASK(2,BUTTON*2)

static volatile uint8_t ESTADO=0;
void confPIN(void);
void confSysTick(void);
void confEINT0(void);

int main(){
    confPIN();
    confEINT0();
    confSysTick();
    while(1){

    }
}
void confPIN(void){
    //LED ROJO P0.22
    LPC_PINCON->PINSEL1 &=~RED_PORT_PIN;
    LPC_PINCON->PINMODE1 &=~RED_PORT_PIN;
    LPC_PINCON->PINMODE1 |= (2<<(RED_LED-16)*2);
    //LED VERDE P3.25 Y LED AZUL P3.26
    LPC_PINCON->PINSEL7 &=~(GREEN_PORT_PIN | BLUE_PORT_PIN);
    LPC_PINCON->PINMODE7 &=~(GREEN_PORT_PIN | BLUE_PORT_PIN);
    LPC_PINCON->PINMODE7 |=(2<<(GREEN_LED-16)*2);
    LPC_PINCON->PINMODE7 |=(2<<(BLUE_LED-16)*2);
    //BOTON P2.10
    LPC_PINCON->PINSEL4 &=~ BUTTON_PORT_PIN;
    LPC_PINCON->PINSEL4 |= BUTTON_PINSEL;
    //SALIDA
    LPC_GPIO0->FIODIR |= RED_PIN;
    LPC_GPIO0->FIOSET = RED_PIN;

    LPC_GPIO3->FIODIR |= (GREEN_PIN|BLUE_PIN);
    LPC_GPIO3->FIOSET = GREEN_PIN;
    LPC_GPIO3->FIOSET = BLUE_PIN;
}
void confEINT0(void){
    LPC_SC->EXTMODE |= (1<<0);
    LPC_SC->EXTPOLAR &=~(1<<0);
    LPC_SC->EXTINT |= (1<<0);
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
void confSysTick(void){
    SysTick->LOAD=(SystemCoreClock/10000)-1;//cada 100us
    SysTick->VAL=0;
    SysTick->CTRL=(1<<0)|(1<<1)|(1<<2);
}
void SysTick_Handler(void){
    static uint8_t PWM=0;
    static uint8_t DUTY=50;
    if(PWM<DUTY){
        switch(ESTADO){
            case 0:
            LPC_GPIO3->FIOSET = GREEN_PIN;
            LPC_GPIO0->FIOCLR = RED_PIN;
            break;
            case 1:
            LPC_GPIO0->FIOSET = RED_PIN;
            LPC_GPIO3->FIOCLR = BLUE_PIN;
            break;
            case 2:
            LPC_GPIO3->FIOSET = BLUE_PIN;
            LPC_GPIO3->FIOCLR = GREEN_PIN; 
            break;
        }
    }else {
        LPC_GPIO0->FIOSET = RED_PIN;
        LPC_GPIO3->FIOSET = GREEN_PIN;
        LPC_GPIO3->FIOSET = BLUE_PIN; 
    }
    PWM++;
    if(PWM==100){
        PWM=0;
    }
}
