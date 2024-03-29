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

#include "rfm12.h"
#include "timer.h"
#include "button.h"

#define BAUD 9600L

#define BAUDR (uint32_t) ((F_CPU / (8 * BAUD)) - 1)
#define BAUDL BAUDR % 256
#define BAUDH BAUDR / 256

int main (void)
{
	LEDDDR |= 1<<LED1 | 1<<LED2 | 1<<LED3;
	LEDPORT |= 1<<LED2;
	
	//prepare UART
	UCSR0A = 1<<U2X0;
	UCSR0B = 1<<TXEN0;
	UCSR0C = 1<<UCSZ01 | 1<<UCSZ00;
	UBRR0H = BAUDH;
	UBRR0L = BAUDL;
	
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
			rfm12.SetWakeupTimer(1000); //rfm12 will wake up �C via IRQ pin after this time if no other event occurs (button, rfm data, rfm bat, ...)
			sleep_mode();
		}
		
		uint8_t rxlen;
		uint8_t *rxdata;
		rxdata = rfm12.GetRX(&rxlen);
		if (rxdata)
		{//rfm12 received a frame, we light a LED for 1 second if 0 < value < 2
			if (rxdata[0] < 3)
			{
				LEDPORT |= 1<<(LED1 + rxdata[0]);
				UDR0 = '1' + rxdata[0];
			}
			timer.SetTime(500);
			rfm12.SetupRX();
		}
		
		uint8_t but = button.Check();
		
		switch (but)
		{
			case BTN_SHORT: //main button pushed, we send a 1 Byte frame and flash a LED
				rfm12.L1Send(&val, 1);
				LEDPORT &= ~(1<<LED1 | 1<<LED2 | 1<<LED3);
				LEDPORT |= 1<<(LED1 + val);
				timer.SetTime(500);
				break;
				
			case BTN_UP://one of the side buttons was pushed, we increase 'val' and flash LED
				if (val < 2)
					val++;
					
				LEDPORT &= ~(1<<LED1 | 1<<LED2 | 1<<LED3);
				LEDPORT |= 1<<(val + LED1);
				timer.SetTime(100);
				break;
				
			case BTN_DOWN://one of the side buttons was pushed, we decrease 'val' and flash LED
				if (val > 0)
					val--;
					
				LEDPORT &= ~(1<<LED1 | 1<<LED2 | 1<<LED3);
				LEDPORT |= 1<<(val + LED1);
				timer.SetTime(100);
				break;
				
			case BTN_UPDOWN_LONG:
				//just to show some button issues, both side buttons were pushed long
				LEDPORT &= ~(1<<LED1 | 1<<LED2 | 1<<LED3);
				LEDPORT |= 1<<LED1 | 1<<LED3;
				timer.SetTime(1000);
				break;
				
			case BTN_LONG: //is fired when main button and both side buttons are still pushed long after power up. can be used e.g. for something like learn mode
				//just to show some button isses
				LEDPORT &= ~(1<<LED1 | 1<<LED2 | 1<<LED3);
				LEDPORT |= 1<<LED2 | 1<<LED3;
				timer.SetTime(2000);
				break;
				
			case BTN_CONFIG: //is fired when main button and both side buttons are still pushed long after power up. can be used e.g. for config mode
				//just to show some button isses
				LEDPORT &= ~(1<<LED1 | 1<<LED2 | 1<<LED3);
				LEDPORT |= 1<<LED1 | 1<<LED2 | 1<<LED3;
				timer.SetTime(2000);
				break;
		}
	}
}