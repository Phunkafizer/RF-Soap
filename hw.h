/*
	hw.h
	Pin definitions
	Stefan Seegel, post@seegel-systeme.de
	feb 2011
	http://opensource.org/licenses/gpl-license.php GNU Public License
*/
#ifndef hw_h
#define hw_h

#define BUTTON_MAIN_PORT PORTD
#define BUTTON_DOWN_PORT PORTD
#define BUTTON_UP_PORT PORTB

#define BUTTON_MAIN_PIN PIND
#define BUTTON_DOWN_PIN PIND
#define BUTTON_UP_PIN PINB

#define BUTTON_MAIN_PINX PD7
#define BUTTON_DOWN_PINX PD3
#define BUTTON_UP_PINX PB7

#define BUTTON_MAIN_SHIFT 7
#define BUTTON_DOWN_SHIFT 2
#define BUTTON_UP_SHIFT 5

#define PWEN_DDR DDRB
#define PWEN_PORT PORTB
#define PWEN_PINX PB0

//Fuse trigger pins
#define FT1_PINX	PD1
#define FT1_PORT	PORTD
#define FT2_PINX	PD0
#define FT2_PORT	PORTD
#define FT3_PINX	PB7
#define FT3_PORT	PORTB

//LED pins
#define LEDDDR		DDRD
#define LEDPORT		PORTD
#define LED1		PD4
#define LED2		PD5
#define LED3		PD6

#endif
