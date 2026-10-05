#include "lpc17xx.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"


#define RED_LED     22
#define GREEN_LED   25
#define BLUE_LED    26
static volatile uint32_t VOLT=0;


void confgADC(void);
void confgDAC(void);


int main(void){
	 confgGPIO();
	 confgADC();
	
    while(1){

    }
}
void confgGPIO(void){
    PINSEL_CFG_Type PinCfg;//
    PinCfg.funcNum =PINSEL_FUNC_1; // funcion 1 del pin
    PinCfg.openDrain = PINSEL_OD_NORMAL; // sin open drain
    PinCfg.pinMode = PINSEL_TRISTATE; // sin pull-up ni pull-down
    PinCfg.pinNum = 23; // pin 23 del puerto 0
    PinCfg.portNum = 0; // puerto 0
    PINSEL_ConfigPin(&PinCfg); // llamo a la funcion que configura el pin
 // -------- ROJO P0.22 --------
    // Función GPIO
    LPC_PINCON->PINSEL1 &= ~(3 << 12);

    // Sin pull-up / pull-down
    LPC_PINCON->PINMODE1 &= ~(3 << 12);
    LPC_PINCON->PINMODE1 |=  (2 << 12);

    // Salida
    LPC_GPIO0->FIODIR |= (1 << RED_LED);


    // -------- VERDE P3.25 --------
    // P3.25 corresponde a bits 18-19 de PINSEL7
    LPC_PINCON->PINSEL7 &= ~(3 << 18);

    // Sin pull-up / pull-down
    LPC_PINCON->PINMODE7 &= ~(3 << 18);
    LPC_PINCON->PINMODE7 |=  (2 << 18);

    // Salida
    LPC_GPIO3->FIODIR |= (1 << GREEN_LED);


    // -------- AZUL P3.26 --------
    // P3.26 corresponde a bits 20-21
    LPC_PINCON->PINSEL7 &= ~(3 << 20);

    // Sin pull-up / pull-down
    LPC_PINCON->PINMODE7 &= ~(3 << 20);
    LPC_PINCON->PINMODE7 |=  (2 << 20);

    // Salida
    LPC_GPIO3->FIODIR |= (1 << BLUE_LED);


    // Apagamos los tres LEDs inicialmente
    LPC_GPIO0->FIOSET = (1 << RED_LED);

    LPC_GPIO3->FIOSET = (1 << GREEN_LED) |
                        (1 << BLUE_LED);
}
void confgADC(void){
    ADC_Init(100000);// Inicia en 100khz
    ADC_ChannelCmd(ADC_CHANNEL_0,ENABLE);//Canal 0
    ADC_IntConfig(ADC_ADINTEN0, ENABLE);
    NVIC_EnableIRQ(ADC_IRQn);
    ADC_BurstCmd(ENABLE);// habilitamos modo rafaga
}
void ADC_IRQHandler(void){
        VOLT=ADC_ChannelGetData(ADC_CHANNEL_0);
        if(VOLT>3723){
            GPIO_ClearPins(0,22);//Led rojo encendido
            GPIO_SetPins(3,25);//apgado verde
            GPIO_SetPins(3,26);//apagado azul
        }else if(VOLT<1240){
            GPIO_ClearPins(3,25);//encendido VERDE
            GPIO_SetPins(0,22);//apgado ROJO
            GPIO_SetPins(3,26);//apagado azul
        }else{
            GPIO_ClearPins(3,25);//encendido VERDE
            GPIO_ClearPins(0,22);//enciende ROJO
            GPIO_SetPins(3,26);//apagado azul
        }

}


/* 
*
*    HECHO POR LUDMILA 
*
*
*
*
*/
/*
 * @file        ADC_DAC_RGB_HW_PWM.c
 * @brief       Muestreo de ADC cada 0.1 s disparado por MAT0.1,
 *              volcado directo al DAC y actualización de LED RGB.
 *              Uso de hardware PWM para el canal Verde (Naranja estable).
 */


#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pwm.h" // Se agrega el driver PWM


// --- Definición de pines ---
#define LED_ROJO    22   // P0.22 (GPIO)
#define LED_VERDE   25   // P3.25 (PWM1.2)
#define LED_AZUL    26   // P3.26 (GPIO)


// --- Umbrales para ADC de 12 bits (Vref = 3.3V) ---
#define THRESHOLD_1V   1241U // 1.0 V
#define THRESHOLD_2V   2482U // 2.0 V
#define THRESHOLD_3V   3723U // 3.0 V


// --- Prototipos de Funciones ---
void confGPIO(void);
void confPWM(void);
void confADC_DAC(void);
void confTimer0(void);
void actualizarLEDs(uint16_t adcVal);


int main(void)
{
    // 1. Inicialización del reloj del sistema (Cortex-M3 a 100 MHz)
    SystemInit();
    SystemCoreClockUpdate();


    // 2. Configuración de periféricos
    confGPIO();
    confPWM();       // Configura P3.25 como salida PWM
    confADC_DAC();
    confTimer0();    // Genera eventos MAT0.1 cada 0.1 segundos para el ADC


    // 3. Modo de bajo consumo absoluto:
    // La CPU duerme. TIMER0 despierta al hardware ADC. El ADC convierte.
    // Al terminar, el ADC despierta a la CPU (ADC_IRQHandler).
    while (1)
    {
        __WFI();
    }
    return 0;
}


void confGPIO(void)
{
    PINSEL_CFG_T pinCfg;
    pinCfg.func      = PINSEL_FUNC_00;      
    pinCfg.mode      = PINSEL_TRISTATE;  
    pinCfg.openDrain = DISABLE;          
   
    // P0.22 (LED Rojo) - Modo GPIO
    pinCfg.port = PORT_0;
    pinCfg.pin  = LED_ROJO;
    PINSEL_ConfigPin(&pinCfg);
    GPIO_SetPinState(PORT_0, LED_ROJO, SET);
    GPIO_SetDir(PORT_0, (1UL << LED_ROJO), GPIO_OUTPUT);


    // NOTA: El pin P3.25 (LED Verde) se elimina de aquí porque lo configurará confPWM()


    // P3.26 (LED Azul) - Modo GPIO
    pinCfg.port = PORT_3;
    pinCfg.pin  = LED_AZUL;
    PINSEL_ConfigPin(&pinCfg);
    GPIO_SetPinState(PORT_3, LED_AZUL, SET);
    GPIO_SetDir(PORT_3, (1UL << LED_AZUL), GPIO_OUTPUT);
}


void confPWM(void)
{
    // 1. Configurar la base de tiempo (1 tick = 1 microsegundo)[cite: 12]
    PWM_TIMERCFG_T timeCfg;
    timeCfg.prescaleOpt = PWM_US;
    timeCfg.prescaleValue = 1;
    PWM_InitTimer(&timeCfg); //


    // 2. Enrutar el pin físico P3.25 internamente a PWM2[cite: 12]
    PWM_PinConfig(PWM2_P3_25); //


    // 3. Configurar el Período Total en MR0 (100 Hz = 10000 us)[cite: 12]
    PWM_MATCHCFG_T matchCfg;
    matchCfg.channel = PWM_MATCH_0;
    matchCfg.intEn = DISABLE;
    matchCfg.stopEn = DISABLE;
    matchCfg.resetEn = ENABLE;    // Reiniciar para crear la onda repetitiva[cite: 12]
    matchCfg.matchValue = 10000;  // Período total de 10000 ticks (10ms)
    PWM_ConfigMatch(&matchCfg); //[cite: 11]


    // 4. Configurar el ciclo inicial en MR2 (Canal 2 = LED Verde)[cite: 12]
    matchCfg.channel = PWM_MATCH_2;
    matchCfg.resetEn = DISABLE;  
    // LÓGICA ACTIVO BAJO:
    // Un valor de 10000 significa que el pin estará en ALTO el 100% del tiempo.
    // Como el LED se enciende con 0V, 10000 = APAGADO.
    matchCfg.matchValue = 10000;  
    PWM_ConfigMatch(&matchCfg); //[cite: 11]


    // 5. Habilitar hardware[cite: 12]
    PWM_ChannelConfig(PWM_CHANNEL_2, PWM_SINGLE_EDGE); //[cite: 11]
    PWM_ChannelEnable(PWM_CHANNEL_2); //[cite: 11]


    PWM_ResetCounter(); //[cite: 11]
    PWM_CounterEnable(); //[cite: 11]
    PWM_Enable(); //[cite: 11]
}


void confADC_DAC(void)
{
    ADC_PinConfig(ADC_CHANNEL_0);
    ADC_Init(200000); // Tasa interna a 200kHz (El disparo lo dicta MAT0.1)
    ADC_ChannelEnable(ADC_CHANNEL_0);


    // Disparo sincronizado por hardware: Flanco de MAT0.1
    ADC_EdgeStartConfig(ADC_START_ON_RISING);
    ADC_StartCmd(ADC_START_ON_MAT01);


    ADC_IntEnable(ADC_INT_CH0);
    NVIC_ClearPendingIRQ(ADC_IRQn);
    NVIC_EnableIRQ(ADC_IRQn);


    DAC_Init();
}


void confTimer0(void)
{
    TIM_TIMERCFG_T timerCfg;
    timerCfg.prescaleOpt   = TIM_US;
    timerCfg.prescaleValue = 1;
    TIM_InitTimer(LPC_TIM0, &timerCfg);


    TIM_MATCHCFG_T matchCfg;
    matchCfg.channel    = TIM_MATCH_1;
    matchCfg.matchValue = 100000UL - 1UL; // 0.1 s
    matchCfg.resetEn    = ENABLE;        
    matchCfg.stopEn     = DISABLE;
    matchCfg.intEn      = DISABLE;        
    matchCfg.extOpt     = TIM_TOGGLE;    
    TIM_ConfigMatch(LPC_TIM0, &matchCfg);


    TIM_Enable(LPC_TIM0);
}


void ADC_IRQHandler(void)
{
    uint16_t adcValue = ADC_ChannelGetData(ADC_CHANNEL_0);
    DAC_UpdateValue(adcValue >> 2);
    actualizarLEDs(adcValue);
}


void actualizarLEDs(uint16_t adcVal)
{
    // 1. Apagar Rojo y Azul vía GPIO (SET = 3.3V)
    GPIO_SetPinState(PORT_0, LED_ROJO, SET);
    GPIO_SetPinState(PORT_3, LED_AZUL, SET);
   
    // 2. Apagar Verde vía PWM.
    // Un match igual al período (10000) mantiene el pin HIGH (Apagado)[cite: 11]
    PWM_MatchUpdateSingle(PWM_MATCH_2, 10000);


    if (adcVal < THRESHOLD_1V)
    {
        // 0V a 1V: VERDE 100%
        // Un match de 0 mantiene el pin LOW el 100% del tiempo[cite: 11]
        PWM_MatchUpdateSingle(PWM_MATCH_2, 0);
    }
    else if (adcVal < THRESHOLD_2V)
    {
        // 1V a 2V: AMARILLO (Rojo 100% + Verde 100%)
        GPIO_SetPinState(PORT_0, LED_ROJO, RESET);
        PWM_MatchUpdateSingle(PWM_MATCH_2, 0); //[cite: 11]
    }
    else if (adcVal < THRESHOLD_3V)
    {
        // 2V a 3V: NARANJA (Rojo 100% + Verde 25%)
        // En Single Edge, el pin es HIGH de 0 a Match, y LOW de Match a 10000.
        // Match en 7500 = HIGH (Apagado) el 75% del tiempo, LOW (Encendido) el 25%.
        GPIO_SetPinState(PORT_0, LED_ROJO, RESET);
        PWM_MatchUpdateSingle(PWM_MATCH_2, 7500); //[cite: 11]
    }
    else
    {
        // > 3V: ROJO 100%
        GPIO_SetPinState(PORT_0, LED_ROJO, RESET);
    }
}
