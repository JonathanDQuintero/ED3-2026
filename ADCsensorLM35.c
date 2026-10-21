#include "lpc17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_nvic.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_systick.h"

#define LM35_1  23 //P0.23
#define LM35_2  24 //P0.24
#define PB1     10 //P2.10
#define PB2     11 //P2.11
#define AOUT    26
#define N       8

static volatile uint8_t CANAL=0;
static volatile uint16_t datoADC0=0;
static volatile uint16_t datoADC1=0;

static uint16_t muestrasADC0[N];
static uint16_t muestrasADC1[N];

static uint8_t  indice0=0;
static uint8_t  indice1=0;

static volatile uint32_t promedioADC0=0;
static volatile uint32_t promedioADC1=0;

static volatile uint16_t V0=0;
static volatile uint16_t V1=0;
static volatile uint16_t diferencia=0;

static uint8_t contadorPB1 = 0;
static uint8_t contadorPB2 = 0;

static uint8_t estadoPB1 = 1;//por que son activos por bajo
static uint8_t estadoPB2 = 1;

static uint32_t salidaDAC=0;
static uint16_t valorDAC=0;


void confPINES(void);
void confADC_DAC(void);
void confTIMER0(void);
void confSysTick(void);


int main(){
    
    confPINES();
    confADC_DAC();
    confSysTick();
    confTIMER0();
    
    
    while(1){
        V0=(promedioADC0*3300)/4095;
        V1=(promedioADC1*3300)/4095;
        //se busca la diferencia entre los 2 datos
        if(V0>V1){
            diferencia=V0-V1;
        }else{
            diferencia=V1-V0;
        }
        if(!estadoPB1 && !estadoPB2){
            salidaDAC=diferencia;
        }else{
            if(!estadoPB1 && estadoPB2 ){
            salidaDAC=V0;  
            }else if(!estadoPB2 && estadoPB1){
            salidaDAC=V1;    
            }else{
            salidaDAC=0;
            }

        //Conversion para cargar los bit al DAC
        valorDAC=(salidaDAC*1024)/3300;
        //Para que no se pase 1024
        if(valorDAC > 1023){
        valorDAC = 1023;
        }
        DAC_UpdateValue(valorDAC);
        }
        
        __WFI();
    }
}

void confPINES(void){
    //PUERTO P0.23
    PINSEL_CFG_Type PinCfg;

    PinCfg.portNum= PINSEL_PORT_0;
    PinCfg.pinNum=  LM35_1;
    PinCfg.funcNum= PINSEL_FUNC_1;
    PinCfg.pinMode= PINSEL_TRISTATE;
    PinCfg.openDrain= PINSEL_OD_NORMAL;

    PINSEL_ConfigPin(&PinCfg);
    //PUERTO P0.24
    PinCfg.pinNum=  LM35_2;
    PINSEL_ConfigPin(&PinCfg);
    //salida del DAC
    PinCfg.pinNum=  AOUT;
    PinCfg.funcNum=PINSEL_FUNC_2;
    PINSEL_ConfigPin(&PinCfg);
    //GPIO de los pulsadores
    PinCfg.portNum=PINSEL_PORT_2;
    PinCfg.pinNum=PB1;
    PinCfg.funcNum=PINSEL_FUNC_0;
    PinCfg.pinMode=PINSEL_PULLUP;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.pinNum=PB2;
    PINSEL_ConfigPin(&PinCfg);
    //configuro como entradas
    GPIO_SetDir(GPIO_PORT_2,1<<PB1,GPIO_INPUT);
    GPIO_SetDir(GPIO_PORT_2,1<<PB2,GPIO_INPUT);
}
void confADC_DAC(void){
    ADC_Init(200000);//inicia el adc con una frecuencia de 200khz
    ADC_BurstDisable();//desabilitamos el modo burst
    ADC_StartCmd(ADC_START_ON_MAT01);//iniciamos la conversion del adc con el timer0 
    ADC_EdgeStartConfig(ADC_START_ON_FALLING);//Conversion por flanco de bajada
    
    ADC_ChannelEnable(ADC_CHANNEL_0);//habilitamos el canal 0 (AD0.0)
    ADC_ChannelDisable(ADC_CHANNEL_1);//Desabilitamos el canal 1 (AD0.1)
    ADC_IntEnable(ADC_INT_GLOBAL);//Hablitacion de interrupcion cuando finaliza la conversion

    NVIC_EnableIRQ(ADC_IRQn);

    DAC_Init();

}
void confTIMER0(void){
    TIM_TIMERCFG_T TimerMode;
    TIM_MATCHCFG_T TimerMatch;

    //configuramos con PCLK=CCLK=100Mhz
    TimerMode.prescaleOpt=TIM_TICK;//Configuramos con ticks por que como tengo que configurar en 5 microseg cuando agrego el valor del match es 2.5microseg y tiene que ser un numero entero
    TimerMode.prescaleValue=10;//Configuramos el prescaler para que cada tick sea de 100ns 

    TimerMatch.channel=TIM_MATCH_1;
    TimerMatch.intEn=DISABLE;
    TimerMatch.stopEn=DISABLE;
    TimerMatch.resetEn=ENABLE;
    TimerMatch.extOpt=TIM_TOGGLE;//Configuramos para que genere un flanco de bajada en el pin MAT0.1
    TimerMatch.matchValue=25;//Configuramos el match para que se genere un flanco de bajada cada 2.5 microsegundos

    TIM_InitTimer(LPC_TIM0,&TimerMode);
    CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_TIMER0,CLKPWR_PCLKSEL_CCLK_DIV_1);//Configuracion para que trabaje con PCLK=CCLK, por que por defecto funciona con CCLK/4
    TIM_ConfigMatch(LPC_TIM0,&TimerMatch);
    TIM_Enable(LPC_TIM0);

}
void confSysTick(void){
    SYSTICK_InternalInit(1);//configurado a 1ms
    SYSTICK_IntCmd(ENABLE);//configuramos la interrupcion del systick
    SYSTICK_Cmd(ENABLE);//habilitamos el systick


}
void SysTick_Handler(void){
    // funcion antirebote
   
    uint8_t lecturaPB1;
    uint8_t lecturaPB2;
    uint32_t puerto2;

    puerto2 = GPIO_ReadValue(GPIO_PORT_2);//lectura del puerto 2

    /* Lectura instantánea */
    if((puerto2 & (1 << PB1)) == 0){// Si el bit PB1 es 0, significa que el pulsador está presionado
        lecturaPB1 = 0;
    }else{
        lecturaPB1 = 1;
    }

    if((puerto2 & (1 << PB2)) == 0){// Si el bit PB2 es 0, significa que el pulsador está presionado
        lecturaPB2 = 0;
    }else{
        lecturaPB2 = 1;
    }


    /* Antirrebote PB1 */
    if(lecturaPB1 != estadoPB1){// Si la lectura instantánea es diferente al estado actual del pulsador

        contadorPB1++;

        if(contadorPB1 >= 20){// Si el contador alcanza 20, significa que el pulsador ha estado estable durante 20 ms
            estadoPB1 = lecturaPB1;
            contadorPB1 = 0;
        }

    }else{
        contadorPB1 = 0;
    }


    /* Antirrebote PB2 */
    if(lecturaPB2 != estadoPB2){

        contadorPB2++;

        if(contadorPB2 >= 20){
            estadoPB2 = lecturaPB2;
            contadorPB2 = 0;
        }

    }else{
        contadorPB2 = 0;
    }
    
}
void ADC_IRQHandler(void){
    static uint16_t sumaADC0=0;
    static uint16_t sumaADC1=0;
    static uint16_t contadorADC0=0;
    static uint16_t contadorADC1=0;
    if(CANAL==0){
        datoADC0=ADC_GlobalGetData();// Se lee los datos del adc0
        //actualizacion de muestras para que haga el promedio con los datos actualizados(podriamos hacer un for)
        sumaADC0=sumaADC0-muestrasADC0[indice0];//se resta la muestra vieja
        muestrasADC0[indice0]=datoADC0;// cargamos en el array la muestra nueva
        sumaADC0=sumaADC0+datoADC0;//se agrega la muestra nueva
        //contador de muestras para el promedio, si es menor a N se incrementa, si es mayor a N se mantiene en N
        if(contadorADC0<N){
            contadorADC0++;
        }

        promedioADC0=sumaADC0/contadorADC0;
        indice0 ++;
        if(indice0>=N){
            indice0=0;
        }
        
        ADC_ChannelDisable(ADC_CHANNEL_0);
        ADC_ChannelEnable(ADC_CHANNEL_1);
        CANAL=1;
    }else{
        datoADC1=ADC_GlobalGetData();
        sumaADC1=sumaADC1-muestrasADC1[indice1];
        muestrasADC1[indice1]=datoADC1;
        sumaADC1=sumaADC1+datoADC1;

        if(contadorADC1<N){
            contadorADC1++;
        }

        promedioADC1=sumaADC1/contadorADC1;
        indice1 ++;
        if(indice1>=N){
            indice1=0;
        }

        ADC_ChannelDisable(ADC_CHANNEL_1);
        ADC_ChannelEnable(ADC_CHANNEL_0);
        CANAL=0;
    }




}