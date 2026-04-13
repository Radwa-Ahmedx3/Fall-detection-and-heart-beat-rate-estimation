/*
 * BIT_MATH.h
 *
 *  Created on: Jul 22, 2025
 *      Author: c.city
 */

#ifndef LIB_BIT_MATH_H_
#define LIB_BIT_MATH_H_

#define SET_BIT(reg,bin)  ((reg)|=(1<<(bin)))
#define TOG_BIT(reg,bin)  ((reg)^=(1<<(bin)))
#define CLR_BIT(reg,bin)  ((reg)&=(~(1<<(bin))))
#define READ_BIT(reg,bin) (((reg)&(1<<bin))>>bin)

#endif /* LIB_BIT_MATH_H_ */
