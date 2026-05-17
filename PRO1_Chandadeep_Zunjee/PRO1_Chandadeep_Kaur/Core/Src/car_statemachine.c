/*
 * car_statemachine.c
 *
 *  Created on: 14 Dec 2025
 *      Author: kaur_
 */
#include <stdint.h>
#include "stm32l4xx_hal.h"
#include "spi.h"
#include "FreeRTOS.h"
#include "gpio.h"
#include <stdbool.h>
#include "leds.h"
#include "functions.h"

#define orangeDelay 2000 	//ms, how long traffic light stays orange
#define greenDelay 5000 	//ms, how long traffic light stays green, IF both lanes have no active cars
#define redDelayMax 6000 	//ms, how long a car can maximum wait on red signal, IF both lanes have active cars

typedef enum {
    VerticalGreen,    			// Vertical GREEN, Horizontal RED
    VerticalOrangeToRed, 		// Vertical ORANGE -> RED, Horizontal RED -> ORANGE
	HorizontalOrangeToGreen,	// Vertical ORANGE -> RED, Horizontal ORANGE -> GREEN

    HorizontalGreen,   			// Horizontal GREEN, Vertical RED
	HorizontalOrangeToRed,		// Horizontal ORANGE -> RED, Vertical RED -> ORANGE
	VerticalOrangeToGreen		// Horizontal ORANGE -> RED, Vertical ORANGE -> GREEN
} states;

void RoadCrossing()
{

    static states State = VerticalGreen;
    static states NextState = VerticalGreen;

    /* Time stamps used for non-blocking timing control */
	static uint32_t startCarGreenTime = 0;	// time when the traffic signal turns green
	static uint32_t startCarOrangeTime = 0;	// time when the car signal turns orange

		State = NextState;

		switch (State) // State machine
			{
				case VerticalGreen:
				car1_SetGreen();	//SW1 & SW3 car signals shall be green
				car2_SetRed();		//SW2 & SW4 car signals shall be red
				//ped1_SetRed();	//all pedestrian signals shall be red
				//ped2_SetRed();

				if(!carHorizontal() && !carVertical() && (HAL_GetTick() - startCarGreenTime >= greenDelay)) { //R2.4

					/* Initialize timing variables for car transition */
					startCarOrangeTime = HAL_GetTick();

					/* If there are no active cars in any direction and greenDelay deadline has passed */
					NextState = VerticalOrangeToRed;

				} else if(carHorizontal() && carVertical() && (HAL_GetTick() - startCarGreenTime >= redDelayMax)) { //R2.6

					/* Initialize timing variables for car transition */
					startCarOrangeTime = HAL_GetTick();

					/* If there are active cars in both lanes, horizontal lane shall wait redDelayMax ms */
					NextState = VerticalOrangeToRed;

				} else if (carHorizontal() && !carVertical()) { //R2.7

					/* Initialize timing variables for car transition */
					startCarOrangeTime = HAL_GetTick();

					/* If there are no active cars in vertical lane but there are active cars in horizontal lane */
					NextState = VerticalOrangeToRed;

				} else //R2.5
					/* if there are active cars in vertical lane but no cars in horizontal lane waiting. Signal remains green. */
					NextState = VerticalGreen;
				break;

				case VerticalOrangeToRed:
					car1_SetOrange();
					car2_SetRed();

					/* Vertical lane stays orange for orangeDelay ms */
					if (HAL_GetTick() - startCarOrangeTime >= orangeDelay) { //turn off orange light

						/* Horizontal lane prepares to turn orange as vertical lane turns red */
						startCarOrangeTime = HAL_GetTick();
						NextState = HorizontalOrangeToGreen;
					}
					break;

				case HorizontalOrangeToGreen:
					car1_SetRed();
					car2_SetOrange();

					if (HAL_GetTick() - startCarOrangeTime >= orangeDelay) {

						/* Horizontal lane prepares to turn green */
						startCarGreenTime = HAL_GetTick();
						NextState = HorizontalGreen;
					}
					break;

				case HorizontalGreen:
					car2_SetGreen();	//SW2 & SW4 car signals shall be green
					car1_SetRed();		//SW1 & SW3 car signals shall be red

					if(!carHorizontal() && !carVertical() && (HAL_GetTick() - startCarGreenTime >= greenDelay)){

						/* Initialize timing variables for car transition */
						startCarOrangeTime = HAL_GetTick();

						/* If there are no active cars in either lanes, signal shall be green for greenDelay ms */
						NextState = HorizontalOrangeToRed;

					} else if(carHorizontal() && carVertical() && (HAL_GetTick() - startCarGreenTime >= redDelayMax)) {

						/* Initialize timing variables for car transition */
						startCarOrangeTime = HAL_GetTick();

						/* If there are active cars in both lanes, vertical lane shall wait redDelayMax ms */
						NextState = HorizontalOrangeToRed;

					} else if(!carHorizontal() && carVertical()) {

						/* Initialize timing variables for car transition */
						startCarOrangeTime = HAL_GetTick();

						/* If there no cars in horizontal lane but active cars in vertical lane */
						NextState = HorizontalOrangeToRed;

					} else {

						/* Active cars in horizontal lane but no cars in vertical lane */
						NextState = HorizontalGreen;
					}
					break;

				case HorizontalOrangeToRed:

					car2_SetOrange();
					car1_SetRed();

					if (HAL_GetTick() - startCarOrangeTime >= orangeDelay) { //turn on orange light

						/* Vertical lane prepares to turn orange as vertical lane turns red */
						startCarOrangeTime = HAL_GetTick();
						NextState = VerticalOrangeToGreen;
					}
				break;

				case VerticalOrangeToGreen:

					car1_SetOrange();
					car2_SetRed();

					if (HAL_GetTick() - startCarOrangeTime >= orangeDelay) {

						/* Horizontal lane prepares to turn green */
						startCarGreenTime = HAL_GetTick();
						NextState = VerticalGreen;
					}
				break;

				default:
					break;
    }
  }




