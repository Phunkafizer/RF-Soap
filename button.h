/*
	button.h
	header file for button class
	Stefan Seegel, post@seegel-systeme.de
	feb 2011
	http://opensource.org/licenses/gpl-license.php GNU Public License
*/
#ifndef button_h
#define button_h

#include "task.h"
#include "timer.h"

#include "hw.h"

#define DEBOUNCE_TIME 20 //ms
#define LONG_TIME 2000 //ms
#define LONG_COUNTER (LONG_TIME / DEBOUNCE_TIME)

//button messages
#define BTN_SHORT 0x01
#define BTN_LONG 0x11
#define BTN_UP 0x04
#define BTN_DOWN 0x02
#define BTN_UPDOWN_LONG 0x16
#define BTN_CONFIG 0x17

class Button: public Task
{
	private:
		uint8_t state;
		uint8_t msg;
		Timer timer;
		bool firstrun;
		uint8_t counter;
	protected:
		bool Execute(void);
	public:
		Button(void);
		uint8_t Check(void);
		void Shutdown(bool led);
};

extern Button button;

#endif