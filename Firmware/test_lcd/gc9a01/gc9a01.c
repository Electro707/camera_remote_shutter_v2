/*
 * GC9A01.c
 *
 *  Created on: Jan 6, 2021
 *      Author: electro
 */

#include "gc9a01.h"
#include "stm32g0xx_ll_utils.h"

gc9a01_color gc9a01_color_white = {.r=0b11111, .g=0b111111, .b=0b11111};
gc9a01_color gc9a01_color_black = {.r=0, .g=0, .b=0};;
gc9a01_color gc9a01_color_red = {.r=0b11111, .g=0, .b=0};;
gc9a01_color gc9a01_color_green = {.r=0, .g=0b111111, .b=0};;
gc9a01_color gc9a01_color_blue = {.r=0, .g=0, .b=0b11111};;
gc9a01_color gc9a01_color_cyan = {.r=0, .g=0b111111, .b=0b11111};;
gc9a01_color gc9a01_color_orange = {.r=0b11111, .g=0b111111, .b=0};;
gc9a01_color gc9a01_color_purple = {.r=0b11111, .g=0, .b=0b11111};;

void _gc9a01_send_command(uint8_t command);
void _gc9a01_send_single_cmd_data(uint8_t command, uint8_t data);
void _gc9a01_send_cmd_and_data(uint8_t command, uint8_t *data, int len);
void _gc9a01_reset(void);

#define GC9A01_COLOR_STRUCT_TO_ARR(_COL, _ARR) _ARR[0] = (_COL.r << 3) | ((_COL.g >> 3) & 0b111); \
		_ARR[1] = ((_COL.g << 5) & 0b111) | _COL.b;

void _gc9a01_send_command(uint8_t command){
	DISP_DC_0;
	DISP_CS_0;
	SPI_Write_Byte(command);

//	while(!LL_SPI_IsActiveFlag_BSY(SPI1));
	DISP_CS_1;
}

void _gc9a01_send_single_cmd_data(uint8_t command, uint8_t data){
	DISP_DC_0;
	DISP_CS_0;
	SPI_Write_Byte(command);
	DISP_DC_1;
	SPI_Write_Byte(data);

//	while(!LL_SPI_IsActiveFlag_BSY(SPI1));
	DISP_CS_1;
}

void _gc9a01_send_cmd_and_data(uint8_t command, uint8_t *data, int len){
	DISP_DC_0;
	DISP_CS_0;
	SPI_Write_Byte(command);
	DISP_DC_1;
	for(int i=0;i<len;i++){
		SPI_Write_Byte(data[i]);
	}

//	while(!LL_SPI_IsActiveFlag_BSY(SPI1));
	DISP_CS_1;
}

void _gc9a01_reset(void){
	DISP_RST_1;
	LL_mDelay(20);
	DISP_RST_0;
	LL_mDelay(50);
	DISP_RST_1;
	LL_mDelay(200);
}

void gc9a01_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1){
	uint8_t dataarray[4];
	dataarray[0] = (x0 & (0xFF<<8))>>8;
	dataarray[1] = x0 & 0xFF;
	dataarray[2] = (x1 & (0xFF<<8))>>8;
	dataarray[3] = x1 & 0xFF;
	_gc9a01_send_cmd_and_data(0x2A,dataarray,4);
	dataarray[0] = (y0 & (0xFF<<8))>>8;
	dataarray[1] = y0 & 0xFF;
	dataarray[2] = (y1 & (0xFF<<8))>>8;
	dataarray[3] = y1 & 0xFF;
	_gc9a01_send_cmd_and_data(0x2B,dataarray,4);
}

void gc9a01_fill_screen(gc9a01_color color){
	uint8_t color_arr[2];
	GC9A01_COLOR_STRUCT_TO_ARR(color, color_arr)
	gc9a01_set_addr_window(0, 0, 240-1, 240-1);

	DISP_DC_0;
	DISP_CS_0;
	SPI_Write_Byte(0x2C);
	DISP_DC_1;
	for(int i=0;i<(240*240);i++){
		SPI_Write_Byte(color_arr[0]);
		SPI_Write_Byte(color_arr[1]);
	}

	DISP_CS_1;
}

void gc9a01_init(void){
	_gc9a01_reset();

	_gc9a01_send_command(DISP_CMD_INTER_REG_EN2);
	_gc9a01_send_single_cmd_data(0xEB, 0x14);

	_gc9a01_send_command(DISP_CMD_INTER_REG_EN1); _gc9a01_send_command(DISP_CMD_INTER_REG_EN2);
	_gc9a01_send_single_cmd_data(0xEB, 0x14);

	_gc9a01_send_single_cmd_data(0x84, 0x40);
	_gc9a01_send_single_cmd_data(0x85, 0xFF);

	_gc9a01_send_single_cmd_data(0x86, 0xFF);
	_gc9a01_send_single_cmd_data(0x87, 0xFF);
	_gc9a01_send_single_cmd_data(0x88, 0x0A);
	_gc9a01_send_single_cmd_data(0x89, 0x21);
	_gc9a01_send_single_cmd_data(0x8A, 0x00);
	_gc9a01_send_single_cmd_data(0x8B, 0x80);
	_gc9a01_send_single_cmd_data(0x8C, 0x01);
	_gc9a01_send_single_cmd_data(0x8D, 0x01);
	_gc9a01_send_single_cmd_data(0x8E, 0xFF);
	_gc9a01_send_single_cmd_data(0x8F, 0xFF);

	_gc9a01_send_cmd_and_data(DISP_CMD_DISPLAY_FUNCTION_CONTROL, (uint8_t[]){0x00, 0x00}, 2);
	_gc9a01_send_single_cmd_data(DISP_CMD_MEMORY_ACCESS_CONTROL, 0x18);

//	writecommand(0x36);	if(USE_HORIZONTAL==0)writedata(0x18);	else if(USE_HORIZONTAL==1)writedata(0x28);	else if(USE_HORIZONTAL==2)writedata(0x48);	else writedata(0x88);

	_gc9a01_send_single_cmd_data(DISP_CMD_PIXEL_FORMAT_SET, 0x05);

	_gc9a01_send_cmd_and_data(0x90, (uint8_t[]){0x08, 0x08, 0x08, 0x08}, 4);

	_gc9a01_send_single_cmd_data(0xBD, 0x06);

	_gc9a01_send_single_cmd_data(0xBC, 0x00);

	_gc9a01_send_cmd_and_data(0xFF, (uint8_t[]){0x60, 0x01, 0x04}, 3);

	_gc9a01_send_single_cmd_data(0xC3, 0x13);
	_gc9a01_send_single_cmd_data(0xC4, 0x13);

	_gc9a01_send_single_cmd_data(0xC9, 0x22);

	_gc9a01_send_single_cmd_data(0xBE, 0x11);

	_gc9a01_send_cmd_and_data(0xE1, (uint8_t[]){0x10, 0x0E}, 2);

	_gc9a01_send_cmd_and_data(0xDF, (uint8_t[]){0x21, 0x0c, 0x02}, 3);

	_gc9a01_send_cmd_and_data(0xF0, (uint8_t[]){0x45, 0x09, 0x08, 0x08, 0x26, 0x2A}, 6);

    _gc9a01_send_cmd_and_data(0xF1, (uint8_t[]){0x43, 0x70, 0x72, 0x36, 0x37, 0x6F}, 6);

    _gc9a01_send_cmd_and_data(0xF2, (uint8_t[]){0x45, 0x09, 0x08, 0x08, 0x26, 0x2A}, 6);
    _gc9a01_send_cmd_and_data(0xF3, (uint8_t[]){0x43, 0x70, 0x72, 0x36, 0x37, 0x6F}, 6);
    _gc9a01_send_cmd_and_data(0xED, (uint8_t[]){0x1B, 0x0B}, 2);
    _gc9a01_send_single_cmd_data(0xAE, 0x77);

    _gc9a01_send_single_cmd_data(0xCD, 0x63);

    _gc9a01_send_cmd_and_data(0x70, (uint8_t[]){0x07, 0x07, 0x04, 0x0E, 0x0F, 0x09, 0x07, 0x08, 0x03}, 9);
    _gc9a01_send_single_cmd_data(0xE8, 0x34);

    _gc9a01_send_cmd_and_data(0x62, (uint8_t[]){0x18, 0x0D, 0x71, 0xED, 0x70, 0x70, 0x18, 0x0F, 0x71, 0xEF, 0x70, 0x70}, 12);
    _gc9a01_send_cmd_and_data(0x63, (uint8_t[]){0x18, 0x11, 0x71, 0xF1, 0x70, 0x70, 0x18, 0x13, 0x71, 0xF3, 0x70, 0x70}, 12);
    _gc9a01_send_cmd_and_data(0x64, (uint8_t[]){0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07}, 7);
    _gc9a01_send_cmd_and_data(0x66, (uint8_t[]){0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00}, 10);
    _gc9a01_send_cmd_and_data(0x67, (uint8_t[]){0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98}, 10);
    _gc9a01_send_cmd_and_data(0x74, (uint8_t[]){0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00}, 7);
    _gc9a01_send_cmd_and_data(0x98, (uint8_t[]){0x3e, 0x07}, 2);
    _gc9a01_send_command(0x35);
    _gc9a01_send_command(0x21);

    _gc9a01_send_command(0x11);    //Exit Sleep
    LL_mDelay(120);
    _gc9a01_send_command(0x29);    //Display on
}
