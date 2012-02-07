/*
	task.h
	header file for task scheduler class
	Stefan Seegel, post@seegel-systeme.de
	feb 2011
	http://opensource.org/licenses/gpl-license.php GNU Public License
*/
#ifndef task_h
#define task_h

#include <avr/io.h>

#define MAX_NUM_TASKS 20

class Task
{
	private:
	protected:
		
	public:
	
		Task(void);
		virtual bool Execute(void) = 0;
	
};

extern bool task_execute(void);

#endif
