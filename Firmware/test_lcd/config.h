#ifndef _CONFIG_H
#define CONFIG_H

#include "stm32g0xx_ll_gpio.h"

#define EVER    ;;

#define PIN_TIA_SEL_1        7      // pa7
#define PIN_TIA_SEL_2        6      // pc6

#define DISP_CS_GPIO_Port   GPIOB
#define DISP_CS_Pin         LL_GPIO_PIN_4

#define DISP_DC_GPIO_Port   GPIOB
#define DISP_DC_Pin         LL_GPIO_PIN_5

#define DISP_RES_GPIO_Port  GPIOB
#define DISP_RES_Pin        LL_GPIO_PIN_6

#endif