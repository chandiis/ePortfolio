/*
 * globals.h
 *
 *  Created on: 8 Dec 2025
 *      Author: kaur_
 */

#ifndef INC_GLOBALS_H_
#define INC_GLOBALS_H_

#include "FreeRTOS.h"
#include "semphr.h"

extern SemaphoreHandle_t xButtonSem;
extern volatile uint8_t pedestrianGreen;


#endif /* INC_GLOBALS_H_ */
