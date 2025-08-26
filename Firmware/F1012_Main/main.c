#include "stm32g0xx.h"
#include "stm32g0xx_ll_rcc.h"
#include "stm32g0xx_ll_system.h"
#include "stm32g0xx_ll_utils.h"
#include "gc9a01.h"
#include "config.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "nanoprintf.h"

void autoShutdownService(void);
void shutdownDevice(void);
void selectTiaSens(uint8_t sens);
void line_draw_abstract(uint8_t *canvasBuff, uint16_t canvasW, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void line_draw_vert_abstract(uint8_t *canvasBuff, uint16_t canvasW, uint16_t x, uint16_t h);

// run-time modes
typedef enum{
	STATE_RESET,		// reset state
	STATE_STANDBY,		// standby, not doing anything with pictures
	STATE_EDITING_VAR,	// editing variable in UI
	STATE_ARMED,		// armed, waiting to take picture
	STATE_TRIG,			// camera triggered
	STATE_SHUTDOWN		// shutdown
}stateMachine_e;

// to keep track of what we are editing through UI
typedef enum{
	UI_EDIT_SHUTTER_TIME,
	UI_EDIT_TRIG_TIME,
	UI_EDIT_TIMELAPSE_N,
	UI_EDIT_TIMELAPSE_DUR,
}uiEditMode_e;

stateMachine_e state;
uint autoShutdownTimer;		// counter for auto shutting down
uint lastEncoderState;
uint lastLcdUpdate;			// last time since we updated the lcd
bool trigUpdateLcd;

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

	// remap PA11 to PA9
	RCC->APBENR2 |= RCC_APBENR2_SYSCFGEN;
	SYSCFG->CFGR1 |= SYSCFG_CFGR1_UCPD2_STROBE | SYSCFG_CFGR1_UCPD1_STROBE;
	// SYSCFG->CFGR1 |= SYSCFG_CFGR1_PA11_RMP;

	// init gpio
	// enable clock for port a, b, and c
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN;

	// PA6 (Trig_1) - output
	// PA7 (CAM_TRIG) - output
	// PA8 (ROT_A) - alternate
	// PA9 (ROT_B, fixed through pcb mod) - alternate
	// PA10 (TIA_SEL2) - output
	// PA11 (3V3_EN_MCU, fixed through pcb mod) - output
	// PA12 (TIA_SEL1) - output
	// PA13 (SWDIO) - alternate
	// PA14 (SWCLK) - alternate
	// PA15 (LCB_BLK) - output
	GPIOA->MODER = 0x695a5fff;
	GPIOA->AFR[1] |= 0x22;              // A8 and A9 -> AF2 (Timer 1 inputs)

	// port b output
	// PB4 (LCD_CS) -> output
	// PB5 (LCD_DC) -> output
	// PB6 (LCD_RES) -> output
	// PB7 (MOSI) -> alternate function
	// PB8 (MISO) -> alternate function
	GPIOB->MODER = 0xFFFE95FF;
	GPIOB->OSPEEDR = 0x0003EA00;        // all high speed for B4 to B6, very high speed for B7-B8
	GPIOB->AFR[0] |= 0x01 << 28;        // B7 -> AF1
	GPIOB->AFR[1] |= 0x01 << 0;         // B8 -> AF1

	// bring all output to reset state
	GPIOA->ODR = 0x800; // pa11
	GPIOB->ODR = 0;
	GPIOC->ODR = 0;

	// init spi, clk/2 (32Mhz), master
	RCC->APBENR1 |= RCC_APBENR1_SPI2EN;
	SPI2->CR2 = 0x0700;
	SPI2->CR1 = 0x0304;

	// init adc
	RCC->APBENR2 |= RCC_APBENR2_ADCEN;
	// Disable ADC in case it was beforehand
	if(ADC1->CR & ADC_CR_ADEN){
		ADC1->CR |= ADC_CR_ADDIS;
		while(ADC1->CR & ADC_CR_ADDIS);
	}

	ADC1->CFGR2 = 0x40000000;       // PCLK/2
	ADC1->SMPR = (0b11 << ADC_SMPR_SMP1_Pos);   // 12.5 ADC clock cycles for sampling time
	ADC1->CHSELR = 1 << 1;          // light as input
	// ADC1->CHSELR = 1 << 3;          // mic as input
	ADC1->CR = ADC_CR_ADVREGEN;      // ADVREGEN=1

	// init timer 1 in quatrature input mode (pa8-ch1 and pa9-ch2)
	RCC->APBENR2 |= RCC_APBENR2_TIM1EN;
	TIM1->SMCR = 0x0003;        // encoder mode 3 (up and down)
	TIM1->CCER = 0x22;      // falling edge
	TIM1->CCMR1 = (0b0110 << TIM_CCMR1_IC1F_Pos) | (0b0110 << TIM_CCMR1_IC1F_Pos) | TIM_CCMR1_CC1S_1 | TIM_CCMR1_CC2S_1;
	TIM1->CR2 = 0x0;
	TIM1->CR1 = 0x1;

	// init timer 14 as the real time interrupt of 1mS
	RCC->APBENR2 |= RCC_APBENR2_TIM14EN;
	TIM14->PSC = 512;
	TIM14->ARR = 125;
	TIM14->CNT = 0;
	TIM14->CR1 = TIM_CR1_URS;
	TIM14->DIER = TIM_DIER_UIE;

	// init timer 6 as a longer timer, period of 10 seconds
	RCC->APBENR1 |= RCC_APBENR1_TIM6EN;
	TIM6->CR1 = TIM_CR1_URS;
	TIM6->DIER = TIM_DIER_UIE;
	TIM6->PSC = 40000;
	TIM6->ARR = 16000;
	TIM6->CNT = 0;

	// set led backlight on
	GPIOA->ODR |= 1 << 15;
	DISP_CS_1;

	NVIC_EnableIRQ(TIM6_DAC_LPTIM1_IRQn);
	NVIC_EnableIRQ(TIM14_IRQn);

	SPI2->CR1 |= SPI_CR1_SPE;
}

void adcCal(void){
	unsigned int cal = 0;
	uint8_t n = 8;
	while(n--){
		ADC1->CR |= ADC_CR_ADCAL;
		while(ADC1->CR & ADC_CR_ADCAL);
		cal += ADC1->CALFACT + 1;
	}
	cal >>= 3;
	ADC1->CALFACT = cal;
}

void initSetupScreen(void){
	uint y = 50;
	gc9a01_fill_screen(gc9a01_color_black);

	gc9a01_print_text_sma("Shutter Time:", 50, y, gc9a01_color_black, gc9a01_color_white, ALIGN_LEFT, NULL);
	y += 16;
	
	gc9a01_print_text_sma("Time To Trig:", 50, y, gc9a01_color_white, gc9a01_color_black, ALIGN_LEFT, NULL);
	y += 16;

	gc9a01_print_text_sma("Timelapse: ", 50, y, gc9a01_color_white, gc9a01_color_black, ALIGN_LEFT, NULL);
	y += 16;

	gc9a01_print_text_sma("N Pics: ", 50, y, gc9a01_color_white, gc9a01_color_black, ALIGN_LEFT, NULL);
	y += 16;

	gc9a01_print_text_sma("Interval: ", 50, y, gc9a01_color_white, gc9a01_color_black, ALIGN_LEFT, NULL);
	y += 16;
}

int main(void)
{
	char nStr[10];

	initMcu();
	autoShutdownTimer = 0;
	lastLcdUpdate = 1000;		// technically infinite
	trigUpdateLcd = false;

	selectTiaSens(0);


	gc9a01_init();


	initSetupScreen();

	TIM1->CNT = 100;
	lastEncoderState = 100;

	TIM14->CR1 |= TIM_CR1_CEN;		// enable timer
	TIM6->CR1 |= TIM_CR1_CEN;		// enable timer
	__enable_irq();

	for(EVER){
		// todo: trigger if encoder is rotated or button pressed to reset auto shutdown timer
		if(trigUpdateLcd){
			if(lastEncoderState != TIM1->CNT){
				lastEncoderState = TIM1->CNT;
				npf_snprintf(nStr, 5, "%04u", (uint)TIM1->CNT);
        		gc9a01_print_text_sma(nStr, 162, 50, 0xFFFF, 0, ALIGN_LEFT, NULL);
				trigUpdateLcd = true;
			}
		}
	}
	return 0;
}

/*
 * real time interrupt triggered by TIMx
 * this runs every 1Khz
 */
void TIM14_IRQHandler(void){
	TIM14->SR = 0;		// clear pending interrupt

	if(trigUpdateLcd == false){
		if(lastLcdUpdate >= 100){
			trigUpdateLcd = true;
		}else{
			lastLcdUpdate++;
		}
	}
}

// called once every 10 seconds
void TIM6_DAC_LPTIM1_IRQHandler(void){
	TIM6->SR = 0;	// clear pending interrupt
	
	autoShutdownTimer += 1;
	if(autoShutdownTimer > AUTO_SHUTDOWN_INTERVAL){
		shutdownDevice();
	}
}

// service to clear the auto-shutdown timer
void autoShutdownService(void){
	autoShutdownTimer = 0;
}

void shutdownDevice(void){
	GPIOA->BRR = 1 << PIN_PSU_ENABLE;
}

/**
 * Selects the amplification amount from the TransImpedance Amplifier circuit
 * Range 0 (disabled) to 3 (max amp)
 */
void selectTiaSens(uint8_t sens){
	switch(sens){
		case 0:
			GPIOA->BRR = (1 << PIN_TIA_SEL_1);          // clear
			GPIOA->BRR = (1 << PIN_TIA_SEL_2);          // clear
			break;
		case 1:
			GPIOA->BSRR = (1 << PIN_TIA_SEL_1);         // set
			GPIOA->BSRR = (1 << PIN_TIA_SEL_2*2);       // clear
			break;
		case 2:
			GPIOA->BSRR = (1 << PIN_TIA_SEL_1*2);       // clear
			GPIOA->BSRR = (1 << PIN_TIA_SEL_2);         // set
			break;
		case 3:
			GPIOA->BSRR = (1 << PIN_TIA_SEL_1);         // set
			GPIOA->BSRR = (1 << PIN_TIA_SEL_2);         // set
			break;
		default:
			// todo: error handling
			break;
	}
}

void line_draw_vert_abstract(uint8_t *canvasBuff, uint16_t canvasW, uint16_t x, uint16_t h){
	int16_t canvasIdx;

	for(int y=0;y<h;y++){
		canvasIdx = (y*canvasW) + x / 8;
		canvasBuff[canvasIdx] |= 1 << (x & 0b111);
	}
}

/**
 * draws a canvas
 * buffer is [Y][X/8], and is bit filled
 */
void line_draw_abstract(uint8_t *canvasBuff, uint16_t canvasW, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1){
	// https://en.wikipedia.org/wiki/Bresenham's_line_algorithm#All_cases
	int16_t dx = abs(x1-x0);
	int16_t dy = -abs(y1-y0);
	int16_t sx = x0 < x1 ? 1 : -1;
	int16_t sy = y0 < y1 ? 1 : -1;
	int16_t error = dx + dy;

	int16_t canvasIdx;

    while(1){
        canvasIdx = (y0*canvasW) + x0 / 8;
        if(canvasIdx < 0){
            canvasIdx = 0;
        }
        canvasBuff[canvasIdx] |= 1 << (x0 & 0b111);

		if((2*error) >= dy){
			if(x0 == x1){break;}
			error += dy;
			x0 += sx;
		}
		if((2*error) <= dx){
			if(y0 == y1){break;}
			error += dx;
			y0 += sy;
		}

	}
}
