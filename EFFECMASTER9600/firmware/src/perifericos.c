#include <stdint.h>
#include "LPC17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_uart.h"
#include "perifericos.h"

void InicializarADC(void)
{
    /*
       ADC PCLK = CCLK = 100 MHz.

       PCLKSEL0 bits 24:25:
       01 = CCLK.
    */
    ADC_Init(200000);
    LPC_SC->PCLKSEL0 &= ~(3UL << 24);
    LPC_SC->PCLKSEL0 |=  (1UL << 24);


    /* P0.23 = AD0.0 */
    ADC_PinConfig(ADC_CHANNEL_0);

    ADC_ChannelEnable(ADC_CHANNEL_0);

    ADC_EdgeStartConfig(ADC_START_ON_RISING);

    /* Disparo por MAT0.1 */
    ADC_StartCmd(ADC_START_ON_MAT01);
    /* Interrumpe en fin de conversion */
    NVIC_EnableIRQ(ADC_IRQn);
    ADC_IntEnable(ADC_INT_CH0);
}


void InicializarDAC(void)
{

    DAC_Init();
    DAC_UpdateValue(512);
}


void InicializarTimer(void)
{

    LPC_SC->PCLKSEL0 &= ~(3UL << 2);
    LPC_SC->PCLKSEL0 |=  (1UL << 2);

    TIM_TIMERCFG_T configuracionTimer;
    TIM_MATCHCFG_T configuracionMatch;

    configuracionTimer.prescaleOpt = TIM_TICK;
    configuracionTimer.prescaleValue = 1;

    TIM_InitTimer(LPC_TIM0, &configuracionTimer);

    /*
       Timer0 PCLK = CCLK = 100 MHz.

       PCLKSEL0 bits 2:3.
    */


    /*
       Match 1:

       100 MHz / 1250 = 80 kHz.

       MAT0.1 conmuta a 80 kHz.
       El flanco ascendente aparece a 40 kHz.
    */

    configuracionMatch.channel = TIM_MATCH_1;

    configuracionMatch.intEn = DISABLE;
    configuracionMatch.stopEn = DISABLE;
    configuracionMatch.resetEn = ENABLE;

    configuracionMatch.extOpt = TIM_TOGGLE;

    configuracionMatch.matchValue = 1250;

    TIM_ConfigMatch(LPC_TIM0, &configuracionMatch);

    TIM_ResetCounter(LPC_TIM0);
}


