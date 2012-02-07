/*
	timer.cpp
	timer class
	Stefan Seegel, post@seegel-systeme.de
	feb 2011
	http://opensource.org/licenses/gpl-license.php GNU Public License
*/
#include "timer.h"
#include <avr/interrupt.h>

volatile static bool temp_idle = true;
volatile bool timers_idle = true;

static struct timer_struct_t *timer_vector = 0;

Timer::Timer(void)
{
	item = new(struct timer_struct_t);
	item->flag = 0;
	item->cnt = 0;
	
	if (timer_vector == 0)
	{//this is the first timer object, init hardware timer
	
		//Init timer for timerobjects
		TCCR0A = 1<<WGM01; //CTC mode
		#if (F_CPU / 256 > 1000)
			#if (F_CPU / 256 / 8 > 1000)
				#if (F_CPU / 256 / 64 > 1000)
					TCCR0B = 1<<CS02;//clk / 256
					OCR0A = F_CPU / 1000 / 256 - 1;
				#else
					TCCR0B = 1<<CS01 | 1<<CS00; //clk / 64
					OCR0A = F_CPU / 1000 / 64 - 1;
				#endif
			#else
				TCCR0B = 1<<CS01; //clk / 8
				OCR0A = F_CPU / 1000 / 8 - 1;
			#endif
		#else
			TCCR0B = 1<<CS00; //clk / 1
			OCR0A = F_CPU / 1000 - 1;
		#endif
		
		TIMSK0 = 1<<OCIE0A;
	}
	
	//put in list
	item->next = timer_vector;
	timer_vector = item;
}


bool Timer::IsFlagged(void)
{
	if (item->flag)
	{	
		item->flag = false;
		return true;
	}
	
	return false;
}

void Timer::SetTime(uint16_t time)
{
	TIMSK0 &= ~(1<<OCIE0A);
	item->cnt = time;
	if (time)
		timers_idle = false;
	
	item->flag = 0;
		
	TIMSK0 |= 1<<OCIE0A;
}

ISR(SIG_OUTPUT_COMPARE0A)
{
	temp_idle = true;
	
	struct timer_struct_t *item = timer_vector;
	
	while (item)
	{
		if (item->cnt)
		{
			temp_idle = false;
			if (--item->cnt == 0)
				item->flag = true;
		}
		item = item->next;
	}
	
	timers_idle = temp_idle;
}
