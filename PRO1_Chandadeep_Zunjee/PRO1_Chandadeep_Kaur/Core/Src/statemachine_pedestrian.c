/*
 * statemachine_pedestrian.c
 *
 *  Created on: 9 Dec 2025
 *      Author: kaur_
 */
/*
 * statemachine_pedestrian.c
 *
 *  Created on: 9 Dec 2025
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

#define toggleFreq 5 //Hz
#define blink_duration 5000 //ms
#define walkingDelay 5000 //ms, how long ped signal is green
#define pedestrianDelay 5000 //ms, how long after the button is pressed that car signal turns red
#define orangeDelay 2000 //ms, how long car signal is orange

uint32_t togglePeriod = 1000 / toggleFreq;

extern uint8_t leds[3];

typedef enum {
	Default,			//default state where ped light is red and traffic light is green
	CarOrangeToGreen, 	//transition red -> orange -> green
	PedBlink,			//pedestrian button pressed, indicator blinks
	PedGreen			//pedestrian green light on
} states;

void PedestrianCrossing1() //This is for pedestrian crossing 1
{

    static states State1 = Default;
    static states NextState1 = Default;

    /* Time stamps used for non-blocking timing control */
	static uint32_t lastToggleTime1 = 0;		// time of last indicator toggle
	static uint32_t startBlinkTime1 = 0;		// time when indicator blinking started
	static uint32_t startPedGreenTime1 = 0;     // time when the pedestrian signal turns green
	static uint32_t startCarRedTime1 = 0;		// time when the car signal turns red
	static uint32_t startCarOrangeTime1 = 0;	// time when the car signal turns orange

	/* Internal flag for indicator LED state */
	static bool indicatorOn1 = false;

		State1 = NextState1;

		switch (State1) // State machine
			{
				case Default:
				car1_SetGreen();	//all car signals shall be green
				car2_SetGreen();
				ped1_SetRed();		//all pedestrian signals shall be red
				ped2_SetRed();

				if (PL1_Hit()) { //if SW5 or Sw6 is pressed
					NextState1 = PedBlink;

					/* Initialize timing variables for blinking and car transition */
					startBlinkTime1 = HAL_GetTick();
					lastToggleTime1 = HAL_GetTick();
					startCarOrangeTime1 = HAL_GetTick();
					indicatorOn1 = false;
				}
				else
					NextState1 = Default; // Stay in Default state
				break;

				case PedBlink:

					/* After orangeDelay, switch car signals from green to orange */
					if (HAL_GetTick() - startCarOrangeTime1 >= orangeDelay) { //turn on orange light
						set(leds, &TL1_Yellow);
						set(leds, &TL3_Yellow);
						reset(leds, &TL1_Green);
						reset(leds, &TL3_Green);
					}

					/* Toggle pedestrian indicator LED based on toggleFreq */
					if (HAL_GetTick() - lastToggleTime1 >= togglePeriod) {
						indicatorOn1 = !indicatorOn1;

						if (indicatorOn1)
						   set(leds, &PL1_Blue);
						else
						   reset(leds, &PL1_Blue);

						lastToggleTime1 = HAL_GetTick();
						startCarRedTime1 = HAL_GetTick();
					}

					/* After blinking duration, allow pedestrians to cross */
					if (HAL_GetTick() - startBlinkTime1 >= blink_duration) {
						reset(leds, &PL1_Blue);	// turn off indicator
						set(leds, &PL1_Green); 	// enable pedestrian green

						startPedGreenTime1 = HAL_GetTick();
						NextState1 = PedGreen;
					}
					break;

				case PedGreen:

					/* Pedestrian light/state is active */
					car1_SetRed();		//cars are stopped with red light
					ped1_SetGreen();	//pedestrians are allowed to pass

					/* After walkingDelay, pedestrians must stop */
					if (HAL_GetTick() - startPedGreenTime1 >= walkingDelay) { //how long the pedestrian light will be green
						ped1_SetRed();
					}

					/* Ensure cars remain red after pedestrian request */
					if (HAL_GetTick() - startCarRedTime1 >= pedestrianDelay) {
						set(leds, &TL1_Red);
						set(leds, &TL3_Red);
						reset(leds, &TL1_Green);
						reset(leds, &TL3_Green);

						startCarOrangeTime1 = HAL_GetTick();
						NextState1 = CarOrangeToGreen;
					}
					break;

				case CarOrangeToGreen:

				// Car stays orange
				reset(leds, &TL1_Red);
				reset(leds, &TL3_Red);
				set(leds, &TL1_Yellow);
				set(leds, &TL3_Yellow);
				reset(leds, &TL1_Green);
				reset(leds, &TL3_Green);

				// After orangeDelay, switch to green
				if (HAL_GetTick() - startCarOrangeTime1 >= orangeDelay) {
					reset(leds, &TL1_Yellow);
					reset(leds, &TL3_Yellow);
					set(leds, &TL1_Green);
					set(leds, &TL3_Green);
					NextState1 = Default;
				}
				break;

				default:
					break;
    }
  }

void PedestrianCrossing2() //This is for pedestrian crossing 2
{
	 static states State2 = Default;
	 static states NextState2 = Default;

	 /* Time stamps used for non-blocking timing control */
	 	static uint32_t lastToggleTime2 = 0;		// time of last indicator toggle
	 	static uint32_t startBlinkTime2 = 0;		// time when indicator blinking started
	 	static uint32_t startPedGreenTime2 = 0;     // time when the pedestrian signal turns green
	 	static uint32_t startCarRedTime2 = 0;		// time when the car signal turns red
	 	static uint32_t startCarOrangeTime2 = 0;	// time when the car signal turns orange

		/* Internal flag for indicator LED state */
		static bool indicatorOn2 = false;

		State2 = NextState2;

		switch (State2) // State machine
		{
			case Default:
				car1_SetGreen();	//all car signals shall be green
				car2_SetGreen();
				ped1_SetRed();		//all pedestrian signals shall be red
				ped2_SetRed();

				if (PL2_Hit()) { //if SW7 or Sw8 is pressed
					NextState2 = PedBlink;

					/* Initialize timing variables for blinking and car transition */
					startBlinkTime2 = HAL_GetTick();
					lastToggleTime2 = HAL_GetTick();
					startCarOrangeTime2 = HAL_GetTick();
					indicatorOn2 = false;
				}
				else
					NextState2 = Default; // Stay in Start state
			break;

			case PedBlink:

			  	/* After orangeDelay, switch car signals from green to orange */
				if (HAL_GetTick() - startCarOrangeTime2 >= orangeDelay) { //turn on orange light
					set(leds, &TL2_Yellow);
					set(leds, &TL4_Yellow);
					reset(leds, &TL2_Green);
					reset(leds, &TL4_Green);
				}

				/* Toggle pedestrian indicator LED based on toggleFreq */
				if (HAL_GetTick() - lastToggleTime2 >= togglePeriod) {
					indicatorOn2 = !indicatorOn2;

					if (indicatorOn2)
					   set(leds, &PL2_Blue);
					else
					   reset(leds, &PL2_Blue);

					lastToggleTime2 = HAL_GetTick();
					startCarRedTime2 = HAL_GetTick();
				}

				/* After blinking duration, allow pedestrians to cross */
				if (HAL_GetTick() - startBlinkTime2 >= blink_duration) {
					reset(leds, &PL2_Blue);	// turn off indicator
					set(leds, &PL2_Green); 	// enable pedestrian green

					startPedGreenTime2 = HAL_GetTick();
					NextState2 = PedGreen;
				}
				break;

		  case PedGreen:

				/* Pedestrian light/state is active */
				car2_SetRed();		//cars are stopped with red light
				ped2_SetGreen();	//pedestrians are allowed to pass

				/* After walkingDelay, pedestrians must stop */
				if (HAL_GetTick() - startPedGreenTime2 >= walkingDelay) { //how long the pedestrian light will be green
					ped2_SetRed();
				}

				/* Ensure cars remain red after pedestrian request */
				if (HAL_GetTick() - startCarRedTime2 >= pedestrianDelay) {
					set(leds, &TL2_Red);
					set(leds, &TL4_Red);
					reset(leds, &TL2_Green);
					reset(leds, &TL4_Green);

					startCarOrangeTime2 = HAL_GetTick();
					NextState2 = CarOrangeToGreen;
				}
				break;

				case CarOrangeToGreen:

				// Car stays orange
				reset(leds, &TL2_Red);
				reset(leds, &TL4_Red);
				set(leds, &TL2_Yellow);
				set(leds, &TL4_Yellow);
				reset(leds, &TL2_Green);
				reset(leds, &TL4_Green);

				// After orangeDelay, switch to green
				if (HAL_GetTick() - startCarOrangeTime2 >= orangeDelay) {
					reset(leds, &TL2_Yellow);
					reset(leds, &TL4_Yellow);
					set(leds, &TL2_Green);
					set(leds, &TL4_Green);
					NextState2 = Default;
				}
				break;

		default:
		break;

	}
 }


