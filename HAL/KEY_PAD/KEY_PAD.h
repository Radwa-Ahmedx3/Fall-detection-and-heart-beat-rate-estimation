/*
 * KEY_PAD.h
 *
 *  Created on: Jul 23, 2025
 *      Author: c.city
 */

#ifndef HAL_KEY_PAD_KEY_PAD_H_
#define HAL_KEY_PAD_KEY_PAD_H_

#define KP_PRESSED      0
#define KP_NOT_PRESSED  255
void KP_init (uint8 KP_Port);

uint8 KP_GetValue (uint8 KP_Port);

#endif /* HAL_KEY_PAD_KEY_PAD_H_ */
