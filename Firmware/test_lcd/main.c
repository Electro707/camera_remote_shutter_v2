#include "stm32g0xx.h"
#include "stm32g0xx_ll_rcc.h"
#include "stm32g0xx_ll_system.h"
#include "stm32g0xx_ll_utils.h"
#include "gc9a01.h"
#include "config.h"

void selectTiaSens(uint8_t sens);

// lv_display_t * display1;

// #define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565))
// static uint8_t buf1[240 * 240 / 10 * BYTES_PER_PIXEL];

void initMcu(void){
    // init clock, 32Mhz system and periferal clock
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
    while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1);

    LL_RCC_HSI_Enable();
    while(LL_RCC_HSI_IsReady() != 1);

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_1, 8, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_Enable();
    LL_RCC_PLL_EnableDomain_SYS();
    while(LL_RCC_PLL_IsReady() != 1);

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_Init1msTick(64000000);
    LL_SetSystemCoreClock(64000000);

    // init gpio
    // enable clock for port a, b, and c
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN;
    // set pa7 to output
    GPIOA->MODER = 0x6bff5fff;
    
    // port b output
    GPIOB->MODER = 0xFFFE95FF;
    GPIOB->OSPEEDR = 0x0003AA00;
    GPIOB->AFR[0] |= 0x01 << 28;
    GPIOB->AFR[1] |= 0x01 << 0;

    GPIOA->ODR = 0;
    GPIOB->ODR = 0;
    GPIOC->ODR = 0;

    // init spi
    RCC->APBENR1 |= RCC_APBENR1_SPI2EN;
    SPI2->CR2 = 0x0700;
    SPI2->CR1 = 0x0304;

    GPIOA->ODR |= 1 << 15;
    DISP_CS_1;

    SPI2->CR1 |= 1 << 6;
}

int main(void)
{
    initMcu();

    selectTiaSens(1);

    gc9a01_init();
    gc9a01_fill_screen(gc9a01_color_red);
    gc9a01_fill_screen(gc9a01_color_green);
    gc9a01_fill_screen(gc9a01_color_blue);

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