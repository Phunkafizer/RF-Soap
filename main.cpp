/*
	main.cpp
	initializing, button handling, rfm handling
	Stefan Seegel, post@seegel-systeme.de
	feb 2011
	http://opensource.org/licenses/gpl-license.php GNU Public License
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "task.h"
#include <util/delay.h>

#include "rfm12.h"
#include "timer.h"
#include "button.h"

#define BAUD 9600L

#define BAUDR (uint32_t) ((F_CPU / (8 * BAUD)) - .5)
#define BAUDL BAUDR % 256
#define BAUDH BAUDR / 256

int main (void)
{
	LEDDDR |= 1<<LED1 | 1<<LED2 | 1<<LED3;
	LEDPORT |= 1<<LED2;
	
	//prepare UART
	/*UCSR0A = 1<<U2X0;
	UCSR0B = 1<<TXEN0;
	UCSR0C = 1<<UCSZ01 | 1<<UCSZ00;
	UBRR0H = BAUDH;
	UBRR0L = BAUDL;*/
	
	sei();
	rfm12.Init();
	rfm12.SetupRX();
	Timer timer;
	timer.SetTime(1000);
	
	bool idle;
	uint8_t val = 0;
	
	while (1)
	{
		idle = !task_execute();
		
		if (rfm12.WakeupFlagged())
		{
			if ((timers_idle))
			{
				//flash middle led
				LEDPORT |= 1<<LED2;
				timer.SetTime(20);
			}
		}
		
		if ((idle) && (timers_idle))
		{
			LEDPORT &= ~(1<<LED1 | 1<<LED2 | 1<<LED3);
			rfm12.SetWakeupTimer(600); //rfm12 will wake up µC via IRQ pin after this time if no other event occurs (button, rfm data, rfm bat, ...)
			sleep_mode();
		}
		
		uint8_t rxlen;
		uint8_t *rxdata;
		rxdata = rfm12.GetRX(&rxlen);
		if (rxdata)
		{
			if (rxdata[0] < 3)
				LEDPORT |= 1<<(LED1 + rxdata[0]);
			timer.SetTime(500);
			rfm12.SetupRX();
		}
		
		
		uint8_t but = button.Check();
		
		switch (but)
		{
			case BTN_SHORT:
				rfm12.L1Send(&val, 1);
				LEDPORT &= ~(1<<LED1 | 1<<LED2 | 1<<LED3);
				LEDPORT |= 1<<(LED1 + val);
				timer.SetTime(500);
				break;
				
			case BTN_UP:
				if (val < 2)
					val++;
					
				LEDPORT &= ~(1<<LED1 | 1<<LED2 | 1<<LED3);
				LEDPORT |= 1<<(val + LED1);
				timer.SetTime(100);
				break;
				
			case BTN_DOWN:
				if (val > 0)
					val--;
					
				LEDPORT &= ~(1<<LED1 | 1<<LED2 | 1<<LED3);
				LEDPORT |= 1<<(val + LED1);
				timer.SetTime(100);
				break;
				
			case BTN_UPDOWN_LONG:
				//just to show some button issues
				LEDPORT &= ~(1<<LED1 | 1<<LED2 | 1<<LED3);
				LEDPORT |= 1<<LED1 | 1<<LED3;
				timer.SetTime(1000);
				break;
				
			case BTN_LONG:
				//just to show some button issues
				LEDPORT &= ~(1<<LED1 | 1<<LED2 | 1<<LED3);
				LEDPORT |= 1<<LED1 | 1<<LED3;
				timer.SetTime(1000);
				break;
				
			case BTN_CONFIG: //is fired when main button is still pushed long after power up. can be used e.g. for config mode
				//just to show some button isses
				LEDPORT |= 1<<LED1 | 1<<LED2 | 1<<LED3;
				timer.SetTime(2000);
				break;
		}
	}
}