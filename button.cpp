/*
	button.cpp
	button class
	Stefan Seegel, post@seegel-systeme.de
	feb 2011
	http://opensource.org/licenses/gpl-license.php GNU Public License
*/
#include "button.h"
#include "rfm12.h"
#include "util/delay.h"
#include <avr/interrupt.h>
#include <avr/sleep.h>

Button button;

ISR (SIG_PIN_CHANGE2){}

Button::Button(void)
{
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	//pull ups for buttons
	BUTTON_MAIN_PORT |= 1<<BUTTON_MAIN_PINX;
	BUTTON_DOWN_PORT |= 1<<BUTTON_DOWN_PINX;
	BUTTON_UP_PORT |= 1<<BUTTON_UP_PINX;
	
	uint8_t d = LONG_TIME / 100;
	while ((bit_is_clear(BUTTON_MAIN_PIN, BUTTON_MAIN_PINX)) && d)
	{
		#ifdef USE_RFM_CLOCK
			_delay_ms(100 * 1000000UL / F_CPU); //CPU runs with 1 MHz here (RFM12 not yet initialized)
		#else
			_delay_ms(100); //CPU runs with internal osc
		#endif
		d--;
	}
	
	if ((d < (LONG_TIME / 100)) && d)
		Shutdown(false); //button was released before LONG_TIME, or button was not pushed at all, i.e. power via USB
		
	//check battery voltage 
	rfm12.Trans(0x820C);
	rfm12.Trans(0xC000 | VBAT_MIN_VAL);
	_delay_ms(50);
	if (rfm12.Trans(0x0000) & 0x0400)
		Shutdown(0); //battery emtpy
	
	//switch on (PWEN)
	//PWEN_DDR |= 1<<PWEN_PINX;
	PWEN_PORT |= 1<<PWEN_PINX;
	
	msg = 0;
	firstrun = true;
	counter = 1;
	
	//Pin change interrupt for main button
	PCICR = 1<<PCIE2;
	PCMSK2 = 1<<PCINT23 | 1<<PCINT19;
	PCMSK0 |= 1<<PCINT7;

	state = 0xF8;
	state |= (BUTTON_MAIN_PIN & 1<<BUTTON_MAIN_PINX) >> BUTTON_MAIN_SHIFT;
	state |= (BUTTON_DOWN_PIN & 1<<BUTTON_DOWN_PINX) >> BUTTON_DOWN_SHIFT;
	state |= (BUTTON_UP_PIN & 1<<BUTTON_UP_PINX) >> BUTTON_UP_SHIFT;
	timer.SetTime(DEBOUNCE_TIME);
}

void Button::Shutdown(bool led)
{
	cli();
	rfm12.Trans(0x8200);
	DDRB = 0;
	PORTB = 0;
	DDRC = 0;
	PORTC = 0;
	
	if (led)
	{
		LEDPORT = 1<<LED2;
		LEDDDR = 1<<LED2;
	}
	else
	{
		PORTD = 0;
		DDRD = 0;
	}
	sleep_mode();
}

bool Button::Execute(void)
{
	uint8_t newstate = 0xF8;
	
	newstate |= (BUTTON_MAIN_PIN & 1<<BUTTON_MAIN_PINX) >> BUTTON_MAIN_SHIFT;
	newstate |= (BUTTON_DOWN_PIN & 1<<BUTTON_DOWN_PINX) >> BUTTON_DOWN_SHIFT;
	newstate |= (BUTTON_UP_PIN & 1<<BUTTON_UP_PINX) >> BUTTON_UP_SHIFT;
	
	bool result = false;
	
	if (newstate != state)
	{
		state = newstate;
		result = true;
		firstrun = false;
		
		counter = 0;
		
		if (state == 0xFF) //button not pushed
			timer.SetTime(0);
		else
			timer.SetTime(DEBOUNCE_TIME);
	}
	
	if (state != 0xFF)
	{//any button pushed!
		result = true;
		
		if (timer.IsFlagged())
		{
			timer.SetTime(DEBOUNCE_TIME);
			
			if (counter == 0) 	//short push
				msg = ~state;
			
			if (counter <= LONG_COUNTER)
				counter++;
			
			if (counter == LONG_COUNTER)
			{
				msg = (~state) | 0x10;
				
				if (!firstrun)
				{
					if (msg == BTN_LONG)
						Shutdown(true);
						
					if (msg == BTN_CONFIG)
						msg = 0;//config only possible immediately after power on
				}
			}
		}
	}
	
	return result;
}

uint8_t Button::Check(void)
{
	uint8_t result = msg;
	msg = 0;
	return result;
}