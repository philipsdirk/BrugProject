/*
 * servo.h - XvR 2020
 */

#ifndef _SERVO_H_
#define _SERVO_H_

// These pins are available on the shield via the header:
//
//		Mega	Uno
// digital 5	PE3	PD5
// digital 6	PH3	PD6
// digital 9	PH6	PB1
// analog 5	PF5	PC5

// The settings below are for the Mega, modify
// in case you want to use other pins
#define PORT_1	PORTA
#define PIN_1	PA0
#define DDR_1	DDRA

#define PORT_2	PORTA
#define PIN_2	PA2
#define DDR_2	DDRA

void init_servo(void);
void servo1_set_percentage(signed char percentage);
void servo2_set_percentage(signed char percentage);

#endif /* _SERVO_H_ */
