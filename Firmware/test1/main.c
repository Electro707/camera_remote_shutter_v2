#include "stm32g0xx.h"
#include "stm32g0xx_ll_rcc.h"
#include "stm32g0xx_ll_system.h"
#include "stm32g0xx_ll_utils.h"

#define EVER    ;;

#define PIN_TIA_SEL_1        7      // pa7
#define PIN_TIA_SEL_2        6      // pc6

void selectTiaSens(uint8_t sens);

void initMcu(void){
    // init clock, 32Mhz system and periferal clock
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
    while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1);

    LL_RCC_HSI_Enable();
    while(LL_RCC_HSI_IsReady() != 1);

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_1, 8, LL_RCC_PLLR_DIV_4);
    LL_RCC_PLL_Enable();
    LL_RCC_PLL_EnableDomain_SYS();
    while(LL_RCC_PLL_IsReady() != 1);

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_Init1msTick(32000000);
    LL_SetSystemCoreClock(32000000);

    // init gpio
    // enable clock for gpio0
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOCEN;
    // set pa7 to output
    GPIOA->MODER &= ~(0b11 << GPIO_MODER_MODE7_Pos);
    GPIOA->MODER |= (0b01 << GPIO_MODER_MODE7_Pos);

    GPIOC->MODER &= ~(0b11 << GPIO_MODER_MODE6_Pos);
    GPIOC->MODER |= (0b01 << GPIO_MODER_MODE6_Pos);

    GPIOA->ODR = 0;
    GPIOC->ODR = 0;
}

int main(void)
{
    initMcu();

    selectTiaSens(1);

    for(EVER){

    }
    return 0;
}

void selectTiaSens(uint8_t sens){
    switch(sens){
        case 0:
            GPIOA->BRR = (1 << PIN_TIA_SEL_1);
            GPIOC->BRR = (1 << PIN_TIA_SEL_2);
            break;
        case 1:
            GPIOA->BSRR = (1 << PIN_TIA_SEL_1);
            GPIOC->BSRR = (1 << PIN_TIA_SEL_2*2);
            break;
        case 2:
            GPIOA->BSRR = (1 << PIN_TIA_SEL_1*2);
            GPIOC->BSRR = (1 << PIN_TIA_SEL_2);
            break;
        case 3:
            GPIOA->BSRR = (1 << PIN_TIA_SEL_1);
            GPIOC->BSRR = (1 << PIN_TIA_SEL_2);
            break;
        default:
            // todo: error handling
            break;
    }
}