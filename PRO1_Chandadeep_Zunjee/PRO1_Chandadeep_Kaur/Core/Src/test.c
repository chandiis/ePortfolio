/*
 * test.c
 *
 *  Created on: 17 Dec 2025
 *      Author: kaur_
 */
#include <stdint.h>
#include "stm32l4xx_hal.h"
#include "spi.h"
#include "FreeRTOS.h"
#include "gpio.h"
#include <stdbool.h>
#include <stdio.h>
#include "leds.h"
#include "functions.h"

extern uint8_t leds[3];

void test_shift_register(void) //test all leds and set and reset function
{
    // Clear all outputs
    for (int i = 0; i < 3; i++) {
        leds[i] = 0x00;
    }
    ShiftReg_SendBytes(leds, 3);
    HAL_Delay(1000);

    // Test one LED at a time
    const defLED* testLEDs[] = {
        &TL1_Red, &TL1_Yellow, &TL1_Green,
        &TL2_Red, &TL2_Yellow, &TL2_Green,
		&TL3_Red, &TL3_Yellow, &TL3_Green,
		&TL4_Red, &TL4_Yellow, &TL4_Green,
        &PL1_Red, &PL1_Green, &PL1_Blue,
        &PL2_Red, &PL2_Green, &PL2_Blue
    };

    for (int i = 0; i < sizeof(testLEDs)/sizeof(testLEDs[0]); i++) {
        set(leds, testLEDs[i]);
        HAL_Delay(700);
        reset(leds, testLEDs[i]);
        HAL_Delay(700);
    }
}

void test_pedestrian_lights(void) {//helper functions to use in FSM
    ped1_SetRed();
    ped2_SetRed();
    HAL_Delay(1000);

    ped1_SetGreen();
    ped2_SetGreen();
    HAL_Delay(1000);
}

void test_trafficlight() { //helper functions to use in FSM

    car1_SetRed();
    HAL_Delay(3000);

    car1_SetOrange();
    HAL_Delay(3000);

    car1_SetGreen();
    HAL_Delay(3000);

    car2_SetRed();
    HAL_Delay(3000);

	car2_SetOrange();
	HAL_Delay(3000);

	car2_SetGreen();
	HAL_Delay(3000);
}

void test_switch_inputs(void) //test for switch/button inputs
{
    // Clear all LEDs
    for (int i = 0; i < 3; i++) leds[i] = 0;
    ShiftReg_SendBytes(leds, 3);

    while (1) {
        if (SW1_Hit()) set(leds, &TL1_Green);
        else           reset(leds, &TL1_Green);

        if (SW2_Hit()) set(leds, &TL2_Green);
        else           reset(leds, &TL2_Green);

        if (SW3_Hit()) set(leds, &TL3_Green);
        else           reset(leds, &TL3_Green);

        if (SW4_Hit()) set(leds, &TL4_Green);
        else           reset(leds, &TL4_Green);

        if (PL1_Hit()) set(leds, &PL1_Blue);
        else           reset(leds, &PL1_Blue);

        if (PL2_Hit()) set(leds, &PL2_Blue);
        else           reset(leds, &PL2_Blue);

        HAL_Delay(50);
    }
}

void test_car_presence(void) //test for the lanes
{
    while (1) {
        if (carVertical()) {
            car1_SetGreen();
        } else {
            car1_SetRed();
        }

        if (carHorizontal()) {
            car2_SetGreen();
        } else {
            car2_SetRed();
        }

        HAL_Delay(100);
    }
}

void Test_program() {

	//test_shift_register();
	//test_pedestrian_lights();
	//test_trafficlight();
	//test_switch_inputs();
	test_car_presence();

}


