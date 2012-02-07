/*
	timer.h
	header file for timer class
	Stefan Seegel, post@seegel-systeme.de
	feb 2011
	http://opensource.org/licenses/gpl-license.php GNU Public License
*/
#ifndef timer_h
#define timer_h

#include <avr/io.h>

struct timer_struct_t
{
	uint16_t cnt;
	bool flag;
	struct timer_struct_t *next;
};

extern volatile bool timers_idle;

class Timer
{
	private:
		struct timer_struct_t *item;

	public:
		Timer(void);
		bool IsFlagged(void);
		void SetTime(uint16_t time);
};

#endif
