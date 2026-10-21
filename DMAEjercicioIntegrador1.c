/**
 * @brief En este ejercicios se pide que mediante le ADC se debe digitalizar una señal cuyo 
 * ancho de banda es de 80khz y su amplitud pico max es 3,3V.
 * Los datos deben ser guardado cada 1 seg utilizando puntero (ADC_POINTER)
 * Los datos deben ser guardado cada 1 seg utilizando el hardware GPDMA(ADC_DMA)
 * Ambos desde la primera mitad de la memoria SRAM ubicada en el bloque AHB SRAM-bank0
 * utilizando un buffer circular para almacenar todos los datos posibles y conservando las ultimas muestras
 * Mediante GPDMA se debe transferir los datos desde la primera mitad de la memoria SRAM(bank 0)
 * a la segunda mitad del la misma memoria. Se deben promediar todos los datos, y dicho valor digitalizado
 * promediado debe ser transmitido por la UART0 en 2 tramas de 1byte que representan la parte entera
 * y decimal del mismo.
 * Frecuencia del cclk 80Mhz para que mediante una interupcion externa en P2.10 por flanco ascendente
 * se comande secuencialmente las funciones ya dichas.
 */
#include "lpc17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_nvic.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_systick.h"

#define PIN 10//P2.10
#define SRAM_ORIGEN 0X2007C000
#define SRAM_DESTINO 0X2007E000
#define N   4096

static uint16_t *ADC_BUFFER=(uint16_t*)SRAM_ORIGEN;
static uint16_t *BUFFER_DMA=(uint16_t*)SRAM_DESTINO;

void confPINES(void);
void confADC_DMA(void);
void confADC(void);
void confEINT0(void);
void confDMA(void);
//void confSysTick(void);


int main(){
    
    confPINES();
    confEINT0();
    confADC();
    confADC_DMA();

    GPDMA_Init();
    
    while(1){
        
        __WFI();
    }
}
void confEINT0(void){
    EXTI_Init();
    EXTI_PinConfig(EXTI_EINT0,EXTI_PULLDOWN);
    //configuracion de la interrupcion
    EXTI_CFG_T cfgEint0={0};
    cfgEint0.line=EXTI_EINT0;
    cfgEint0.mode=EXTI_EDGE_SENSITIVE;
    cfgEint0.polarity=EXTI_RISING_EDGE;
    
    EXTI_Config(&cfgEint0);
    EXTI_EnableIRQ(EXTI_EINT0);

}
void confADC(void){
    ADC_Init(160000);// La consigna pide un ancho de banda de 80khz, por lo que la frecuencia de muestreo debe ser el doble, es decir 160khz
    ADC_PinConfig(ADC_CHANNEL_0);
    ADC_ChannelEnable(ADC_CHANNEL_0)
    ADC_BurstEnable();
    ADC_StartCmd(ADC_START_CONTINUOUS);
    ADC_IntEnable(ADC_INT_GLOBAL);

    NVIC_EnableIRQ(ADC_IRQn);

}
void confADC_DMA(void){
    
   static GPDMA_LLI_T LLI_ADC;

    LLI_ADC.srcAddr=(uint32_t)&LPC_ADC->ADGDR;
    LLI_ADC.dstAddr=(uint16_t)ADC_BUFFER;
    LLI_ADC.nextLLI=0;// por que quiero despues pasar a otro canal de DMA que trasfiera de ADC_BUFFER a BUFFER_DMA, por eso pongo 0 
    //para que no haga nada mas
    LLI_ADC.control=4095
                   |(0<<15)// recibo de a 1 dato
                   |(1<<18)//transferencia fuente de media palabra
                   |(1<<21)//transferenciad destino de media palabra
                   |(0<<26)//no incrementa despues de cada transfe en la fuente
                   |(1<<27)//incrementa despues de cada transfe en el destino
                   |(1<<31);//hablito interrupcion

    GPDMA_Channel_CFG_T cfgADC;

    cfgADC.channelNum=GPDMA_CH_0;
    cfgADC.type=GPDMA_P2M;
    cfgADC.transferSize=4095;
    cfgADC.srcMemAddr=(uint32_t)&LPC_ADC->ADGDR;// no es neceario por que al poner GPDMA_ADC el driver escribo dentro del driver. 
    //Se lo pone por fines de saber de donde viene (en la tabla de lpc17xx_gpdma.C sale esto)
    cfgADC.dstMemAddr=(uint16_t)ADC_BUFFER;
    cfgADC.srcConn=GPDMA_ADC;
    cfgADC.dstConn=0;// Como es de P2M no se usa

    cfgADC.src.width=GPDMA_HALFWORD;// Por que el ADC es de 12bits, puedo poner de una palabra pero ocupo lugar innecesario(hay que cuidar el espacio)
    cfgADC.src.burst=1;// para que tome de a un solo dato, si por ej: pongo 4 el DMA va a tomar 4 veces el mismo valor
    cfgADC.src.increment=DISABLE;//POR QUE ES DE P2M NO SE INCREMENTA
    
    cfgADC.dst.width=GPDMA_HALFWORD;//
    cfgADC.dst.burst=GPDMA_BSIZE_1;
    cfgADC.dst.increment=ENABLE;//POR QUE SE INCREMENTA EN EL DESTINO
    
    cfgADC.intTC=ENABLE;
    cfgADC.intErr=DISABLE;//Interrupcion por error no quiero por que no me interesa en este caso
    cfgADC.linkedList=&LLI_ADC;
    GPDMA_SetupChannel(&cfgADC);
    GPDMA_ChannelStart(GPDMA_CH_0)
}
void confDMA(void){
    static GPDMA_LLI_T LLI_DMA;
    LLI_DMA.srcAddr=(uint16_t)&ADC_BUFFER;
    LLI_DMA.dstAddr=(uint16_t)&BUFFER_DMA;
    LLI_DMA.nextLLI=0;
    LLI_DMA.ctrl=4095
                 |(0<<12)
                 |(0<<15)
                 |(1<<18)
                 |(1<<21)
                 |(1<<26)
                 |(1<<27)
                 |(1<<31);

    GPDMA_Channel_CFG_T cfgDMA;
    cfgDMA.channelNum=GPDMA_CH_1;
    cfgDMA.type=GPDMA_M2M;
    cfgDMA.transferSize=4095;
    cfgDMA.srcMemAddr=(uint16_t)ADC_BUFFER;
    cfgDMA.dsrMemAddr=(uint16_t)BUFFER_DMA;
    
    cfgDMA.src.width=GPDMA_HALFWORD;
    cfgDMA.src.burst=GPDMA_BSIZE_1;
    cfgDMA.src.increment=ENABLE;
    cfgDMA.dst.width=GPDMA_HALFWORD;
    cfgDMA.dst.burst=1;
    cfgDMA.dst.increment=ENABLE;

    cfgDMA.intTC=ENABLE;
    cfgDMA.intErr=ENABLE;

    cfgDMA.linkedList=&LLI_DMA;
    GPDMA_SetupChannel(&cfgDMA);
    GPDMA_ChannelStart(GPDMA_CH_1);
}
void DMA_IRQHandler(void){
    //termino el canal 0 (ADC a BANK 0)
    if(GPDMA_IntGetStatus(GPDMA_INTTC,GPDMA_CH_0)){
        //limpiamos la bandera
        GPDMA_ClearIntPending(GPDMA_INTTC)
        //Reconfigura e inicia la transferencia del canal 1(bank0 a bank1)
        confDMA();
        
    }else if(GPDMA_IntGetStatus(GPDMA_INTTC,GPDMA_CH_1)){
        //Limpiamo la bandera de la interrupcion
        GPDMA_ClearIntPending(GPDMA_INTTC)
        //Reconfiguro e inicia la transferencia del canal0(ADC a BANK 0)
        confADC_DMA();
    }
}

