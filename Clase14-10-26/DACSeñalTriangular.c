/** 
*@brief   utilizando el timer0, un DAC, interrupciones y el driver de la lpc1769, escribir un codigo que
*         permita generar una señal triangular periodica simetrica, que tenga el minimo periodo posible
*         la maxima excursion de voltaje pico a pico posibe y el minimo incremento de señaL posible por
*         el DAC. Suponer una frecuencia de cclk de 100Mhz. El codigo debidamente comentado
*
*@date    minimo periodo =>max velocidad de actualizacion del DAC
*         maxima excursion=>min=0, max=1023
*         minimo incremento=>1, todos los valores entre 0 y 1023
*/

#include "lpc17xx.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_nvic.h"

#define  DAC_MAX_VALUE 1023

static volatile uint16_t valorDAC=0;
static volatile uint8_t  dacStep=0;

void confPINES(void);
void confDAC(void);
void confTIMER0(void);


int main(){
    confPINES();
    confDAC();
    confTIMER0();
    while(1){
        __WFI();
    }
}
void confDAC(void){
    DAC_Init();
    DAC_UpdateValue(valorDAC);
}
void confTIMER0(void){
    TIM_TIMERCFG_T timercfg = {TIM_US,1};

    TIM_MATCHCFG_T matchcfg = {
    .channel =  TIM_MATCH_0,
    .intEn = ENABLE,
    .resetEn= ENABLE,
    . 
    }
    TIM_InitTimer(LPC_TIM0,&timercfg);
    TIM_ConfigMatch(LPC_TIM0,&matchcfg);

    NVIC_ClearPendingIRQ(TIMER0_IRQn);
    NVIC_EnableIRQ(TIMER0_IRQn);
    TIM_Enable(LPC_TIM0);
}
void TIMER0_IRQHandler{
    DAC_UpdateValue(valorDAC);

    if (valorDAC>=DAC_MAX_VALUE){
        dacStep=-1;
    }else if(valorDAC==0){
        dacStep=1;
    }
    valorDAC+= dacStep;
    TIM_ClearIntPending(LPC_TIM0,TIM_MR0_INT);
}


/**
 * @brief Programar el microcontrolador lpc1769 para que mediante su ADC digitalice dos señales analogicas
 *        cuyos anchos de bandas son de 10Khz cada uno. Los canales utilizados deben ser el 2 y el 4 
 *        y los datos deben ser guardados en 2 regiones de memorias distintas que permitan contar con los
 *        ultimos 2 digitos de cada canal. Suponer una frecuencia de cclk de 100Mhz.
 *        El codigo debe estar debidamente comentado.
 * 
 */

#include "lpc17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_nvic.h"

#define ADC_SAMPLE_RATE 5000
#define ADC_BUFFER_SIZE 20

#define BUFFER_CH2((volatile uint16_t*)0x2007C00U)// AHB SRAM BANK0
#define BUFFER_CH4((volatile uint16_t*)0x2008000U)// AHB SRAM BANK1

static volatile uint16_t* buffer_ch2= BUFFER_CH2;
static volatile uint16_t* buffer_ch4= BUFFER_CH4;

static volatile uint8_t idx=0U; //buffer index

void confADC_DAC(void);


int main(){
    confADC_DAC();
    while(1){
        __WFI();
    }
}
void confADC_DAC(void){
    ADC_Init(ADC_SAMPLE_RATE)//frecuencia de trabajo 50khz por que son 2 canales y cada canal 2*10khz Y por Nyquist tiene que ser mayor o igual a 40khz
    ADC_PinConfig(ADC_CHANNEL_2);
    ADC_PinConfig(ADC_CHANNEL_4);
    ADC_IntEnable(ADC_INT_CH2);
    ADC_IntEnable(ADC_INT_CH4);

    NVIC_ClearPendingIRQ(ADC_IRQn);
    NVIC_EnableIRQ(ADC_IRQn);

    ADC_ChannelEnable(ADC_CHANNEL_2);
    ADC_StartCmd(ADC_START_NOW);
}
void ADC_IRQHandler(void){
    if(ADC_ChannelGetStatus(ADC_CHANNEL_2, ADC_DATA_DONE)){
        buffer_ch2[idx]=ADC_ChannelGetData(ADC_CHANNEL_2);
        ADC_ChannelDisable(ADC_CHANNEL_2);
        ADC_ChannelEnable(ADC_CHANNEL_4);
        ADC_StartCmd(ADC_START_NOW);
    }else {
        buffer_ch4[idc]=ADC_ChannelGetData(ADC_CHANNEL_4);
        idx=(idx+1U)%ADC_BUFFER_SIZE;
        ADC_ChannelDisable(ADC_CHANNEL_4);
        ADC_ChannelEnable(ADC_CHANNEL_2);
        ADC_StartCmd(ADC_START_NOW);
    }
}
/**
 * @brief 
*           En la siguiente sección de código se realiza la configuración de un canal de DMA
* a - Explique detalladamente cómo queda configurado el canal, que tipo de transferencia está realizando.
* b - ¿Qué datos se transfieren, de qué posición a cuál y cuántas veces?
* c- ¿Cómo se define el tiempo de "Interrup DMA request"  o el tiempo de transferencia de c/dato?
*
*/
void confDMA(void){
    GPDMA_LLI_Type DMA_LLI_Struct1;
    GPDMA_LLI_Type DMA_LLI_Struct2;

    DMA_LLI_Struct1.SrcAddr  = (uint32_t)(una_tabla_global);
    DMA_LLI_Struct1.DstAddr  = (uint32_t)(otra_tabla_global);
    DMA_LLI_Struct1.NextLLI  = (uint32_t)(&DMA_LLI_Struct2);

    DMA_LLI_Struct2.SrcAddr  = (uint32_t)(otra_tabla_global);
    DMA_LLI_Struct2.DstAddr  = (uint32_t)(una_tabla_global);
    DMA_LLI_Struct2.NextLLI  = (uint32_t)(&DMA_LLI_Struct1);

    DMA_LLI_Struct.Control = TAMAÑO_DE_TABLAS    // DECLARADA POR DEFINE
        | (2<<18)
        | (2<<21)
        | (1<<27)
        | (1<<26)
        ;

    GPDMA_Init();

    GPDMACfg.ChanneNum      = 0;
    GPDMACfg.SrcMemAddr     = (uint32_t)(una_tabla_global);
    GPDMACfg.DstMemAddr     = (uint32_t)(otra_tabla_global);
    GPDMACfg.TransferSize   = TAMAÑO_DE_TABLAS;
    GPDMACfg.TransferWidth  = 0;
    GPDMACfg.TransferType   = GPDMA_TRANSFERTYPE_M2M;
    GPDMACfg.SrcConn        = 0;
    GPDMACfg.DstConn        = 0;
    GPDMACfg.DMALLI         = (uint32_t)(&DMA_LLI_Struct2);

    GPDMA_Setup(&GPDMACfg);

    return;
}

