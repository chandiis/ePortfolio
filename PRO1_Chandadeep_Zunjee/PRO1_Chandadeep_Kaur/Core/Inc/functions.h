/*
 * functions.h
 *
 *  Created on: Dec 2, 2025
 *      Author: kaur_
 */

#ifndef INC_FUNCTIONS_H_
#define INC_FUNCTIONS_H_
#include <stdint.h>
#include <stdbool.h>
#include "leds.h"

void ShiftReg_SendBytes(uint8_t data[], uint16_t length);
bool PL1_Hit(void);
bool PL2_Hit(void);
bool SW1_Hit(void);
bool SW2_Hit(void);
bool SW3_Hit(void);
bool SW4_Hit(void);
void set(uint8_t leds[], const defLED *led);
void reset(uint8_t leds[], const defLED *led);
void car1_SetGreen(void);
void car1_SetOrange(void);
void car1_SetRed(void);
void ped1_SetRed(void);
void ped1_SetGreen(void);
void car2_SetGreen(void);
void car2_SetOrange(void);
void car2_SetRed(void);
void ped2_SetRed(void);
void ped2_SetGreen(void);
bool carVertical();
bool carHorizontal();

#endif /* INC_FUNCTIONS_H_ */
