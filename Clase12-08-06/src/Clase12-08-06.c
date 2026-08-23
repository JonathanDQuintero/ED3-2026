#include "LPC17xx.h"

#define DELAY 2500
#define RED_LED (22)

// hacer parpadear del pin 0.22 (led rojo en la LPC)
void confPCB(); //funcion de configuracion de pines
void delay();
void SysTick_Handler();
int main(){
	confPCB();
	SysTick_Handler();
	while(1){
		LPC_GPIO0 -> FIOCLR = 1 << RED_LED;// Enciendo el led (el led enciende con un cero por que es activo por bajo el led segun el datasheet)
		//LPC_GPIO -> FIOCLR |= 1<<22;
		delay();
		LPC_GPIO0 -> FIOSET = 1 << RED_LED;// apago el led
		//LPC_GPIO -> FIOSET |= 1<<22;
		delay();
	}

}

void confPCB(){
	LPC_PINCON -> PINSEL1 &= ~(3<<12);// Los 2 bits los estoy forzondo a 0 (3dec = 11 binario) por eso usamos "~" para negarlo. Esto lo hacemos para no hacer una and
	//LPC_PINCON -> PINSEL1 &= ~(1<<12 | 1<<13);

	LPC_PINCON -> PINMODE1 &= ~(1<<12);
	LPC_PINCON -> PINMODE1 |= (1<<13); // poniendo a 1

	LPC_GPIO0 -> FIODIR |= 1<< 22;

	LPC_GPIO0 -> FIOSET = 1<< 22; // se hacen solo con igual sin & o |
}

void SysTick_Handler(void)
	{
	int i;
	 i--;
	 if(i==0)
	 {
	 i=100;
	 if ((1<<22)&LPC_GPIO0->FIOPIN)
	 {
	 LPC_GPIO0->FIOCLR=1<<22;
	 }
	 else
	 {
	 LPC_GPIO0->FIOSET=1<<22;
	 }
	 }
	}
