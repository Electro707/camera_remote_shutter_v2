/*
 * GC9A01.h
 *
 *  Created on: Jan 6, 2021
 *      Author: electro
 */

#ifndef INC_GC9A01_H_
#define INC_GC9A01_H_

#include <stdint.h>
#include "stm32g0xx.h"
#include "stm32g0xx_ll_gpio.h"
#include "stm32g0xx_ll_spi.h"
#include "config.h"

typedef union{
	uint16_t color;
	struct{
		uint16_t 	r: 5,
					g: 6,
					b: 5;
	};
} gc9a01_color;

/********** Function Declaration **********/
void gc9a01_init(void);
void gc9a01_fill_screen(gc9a01_color color);
void gc9a01_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/********** Defines for Commands and Whatnot **********/
#define DISP_CMD_INTER_REG_EN2 0xEF
#define DISP_CMD_INTER_REG_EN1 0xFE
#define DISP_CMD_DISPLAY_FUNCTION_CONTROL 0xB6
#define DISP_CMD_MEMORY_ACCESS_CONTROL 0x36
#define DISP_CMD_PIXEL_FORMAT_SET 0x3A

/********** Macros **********/
//#define DISP_CS_0     DISP_CS_GPIO_Port->ODR &= ~(DISP_CS_Pin)
//#define DISP_CS_1     DISP_CS_GPIO_Port->ODR |= (DISP_CS_Pin)
//
//#define DISP_DC_0     DISP_DC_GPIO_Port->ODR &= ~(DISP_DC_Pin)
//#define DISP_DC_1     DISP_DC_GPIO_Port->ODR |= (DISP_DC_Pin)
//
//#define DISP_RST_0     DISP_RES_GPIO_Port->ODR &= ~(DISP_RES_Pin)
//#define DISP_RST_1     DISP_RES_GPIO_Port->ODR |= (DISP_RES_Pin)

#pragma GCC push_options
#pragma GCC optimize ("O0")

#define DISP_CS_0     LL_GPIO_ResetOutputPin(DISP_CS_GPIO_Port, DISP_CS_Pin)
#define DISP_CS_1     LL_GPIO_SetOutputPin(DISP_CS_GPIO_Port, DISP_CS_Pin)

#define DISP_DC_0     LL_GPIO_ResetOutputPin(DISP_DC_GPIO_Port, DISP_DC_Pin)
// #define DISP_DC_1     LL_GPIO_SetOutputPin(DISP_DC_GPIO_Port, DISP_DC_Pin)
#define DISP_DC_1     GPIOB->BSRR = DISP_DC_Pin

#define DISP_RST_0     LL_GPIO_ResetOutputPin(DISP_RES_GPIO_Port, DISP_RES_Pin)
#define DISP_RST_1     LL_GPIO_SetOutputPin(DISP_RES_GPIO_Port, DISP_RES_Pin)

// #define SPI_Write_Byte(__DATA) SPI2->DR = __DATA; while( (SPI2->SR & (1<<7)) != 0)
// #define SPI_Write_Byte(__DATA) LL_SPI_TransmitData8(SPI2, __DATA); while(LL_SPI_IsActiveFlag_BSY(SPI2))
#define SPI_Write_Byte(__DATA) LL_SPI_TransmitData8(SPI2, __DATA); while( (SPI2->SR & (1<<7)) != 0)


extern gc9a01_color gc9a01_color_white;
extern gc9a01_color gc9a01_color_black;
extern gc9a01_color gc9a01_color_red;
extern gc9a01_color gc9a01_color_green;
extern gc9a01_color gc9a01_color_blue;
extern gc9a01_color gc9a01_color_cyan;
extern gc9a01_color gc9a01_color_orange;
extern gc9a01_color gc9a01_color_purple;

#pragma GCC pop_options


#endif /* INC_GC9A01_H_ */
