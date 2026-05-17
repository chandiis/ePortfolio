
#include <stdint.h>
#include "stm32l4xx_hal.h"
#include "spi.h"
#include "FreeRTOS.h"
#include "gpio.h"
#include <stdbool.h>
#include <stdio.h>
#include "leds.h"

extern uint8_t leds[3];

// data[] => an array with bytes that will be sent to the shift register
// length = how many bytes the array has
void ShiftReg_SendBytes(uint8_t data[], uint16_t length)
{
    // Skicka N bytes över SPI
    HAL_SPI_Transmit(&hspi3, data, length, HAL_MAX_DELAY);

    // Latch
    HAL_GPIO_WritePin(STCP_GPIO_Port, STCP_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(STCP_GPIO_Port, STCP_Pin, GPIO_PIN_RESET);
}

bool PL1_Hit(void) //SW5 and SW6
{ if (HAL_GPIO_ReadPin(PL1_Switch_GPIO_Port, PL1_Switch_Pin) == 0) return true;
else return false;
}

bool PL2_Hit(void) //SW7 and SW8
{ if (HAL_GPIO_ReadPin(PL2_Switch_GPIO_Port, PL2_Switch_Pin) == 0) return true;
else return false;
}

bool SW1_Hit(void) //car for traffic light 1
{ if (HAL_GPIO_ReadPin(TL1_Car_GPIO_Port, TL1_Car_Pin) == 0) return true;
else return false;
}

bool SW2_Hit(void) //car for traffic light 2
{ if (HAL_GPIO_ReadPin(TL2_Car_GPIO_Port, TL2_Car_Pin) == 0) return true;
else return false;
}

bool SW3_Hit(void) //car for traffic light 3
{ if (HAL_GPIO_ReadPin(TL3_Car_GPIO_Port, TL3_Car_Pin) == 0) return true;
else return false;
}

bool SW4_Hit(void) //car for traffic light 4
{ if (HAL_GPIO_ReadPin(TL4_Car_GPIO_Port, TL4_Car_Pin) == 0) return true;
else return false;
}

void set(uint8_t leds[], const defLED *led){
	leds[led->shift_reg] = leds[led->shift_reg] | (1 << led->bits);
	//leds[0] = leds[0] | (1 << 0) = leds[0] | (00000001)
	ShiftReg_SendBytes(leds, 3);
}

void reset(uint8_t leds[], const defLED *led){
	leds[led->shift_reg] = leds[led->shift_reg] & ~(1 << led->bits); //~ = bitwise NOT
	ShiftReg_SendBytes(leds, 3);
}

void car1_SetGreen(void){ //function to set trafficlight 1 green
    reset(leds, &TL1_Red);
    reset(leds, &TL1_Yellow);
    set(leds, &TL1_Green);

    reset(leds, &TL3_Red);
    reset(leds, &TL3_Yellow);
    set(leds, &TL3_Green);
}

void car1_SetOrange(void){ //function to set trafficlight 1 orange
	reset(leds, &TL1_Green);
	reset(leds, &TL1_Red);
	set(leds, &TL1_Yellow);

	reset(leds, &TL3_Green);
	reset(leds, &TL3_Red);
	set(leds, &TL3_Yellow);
}

void car1_SetRed(void){ //function to set trafficlight 1 red
    reset(leds, &TL1_Green);
    reset(leds, &TL1_Yellow);
    set(leds, &TL1_Red);

    reset(leds, &TL3_Green);
    reset(leds, &TL3_Yellow);
    set(leds, &TL3_Red);
}

void ped1_SetRed(void){ //function to turn on ped1 red light
    set(leds, &PL1_Red);
    reset(leds, &PL1_Green);
    reset(leds, &PL1_Blue);
}

void ped1_SetGreen(void){ //function to turn of ped1 green light
    reset(leds, &PL1_Red);
    set(leds, &PL1_Green);
    reset(leds, &PL1_Blue);
}

void car2_SetGreen(void) { //function to set trafficlight 2 green
    reset(leds, &TL2_Red);
    reset(leds, &TL2_Yellow);
    set(leds, &TL2_Green);

    reset(leds, &TL4_Red);
    reset(leds, &TL4_Yellow);
    set(leds, &TL4_Green);
}

void car2_SetOrange(void){ //function to set trafficlight 2 orange
	reset(leds, &TL2_Green);
	reset(leds, &TL2_Red);
	set(leds, &TL2_Yellow);

	reset(leds, &TL4_Green);
	reset(leds, &TL4_Red);
	set(leds, &TL4_Yellow);
}

void car2_SetRed(void){ //function to set trafficlight 2 red
    reset(leds, &TL2_Green);
    reset(leds, &TL2_Yellow);
    set(leds, &TL2_Red);

    reset(leds, &TL4_Green);
    reset(leds, &TL4_Yellow);
    set(leds, &TL4_Red);
}

void ped2_SetRed(void){ //function to turn on ped2 red light
    set(leds, &PL2_Red);
    reset(leds, &PL2_Green);
    reset(leds, &PL2_Blue);
}

void ped2_SetGreen(void){ //function to turn of ped2 green light
    reset(leds, &PL2_Red);
    set(leds, &PL2_Green);
    reset(leds, &PL2_Blue);
}

bool carVertical(){
	return SW1_Hit() || SW3_Hit();
}

bool carHorizontal() {
	return SW2_Hit() || SW4_Hit();
}

