/*
	task.cpp
	task scheduler class
	Stefan Seegel, post@seegel-systeme.de
	feb 2011
	http://opensource.org/licenses/gpl-license.php GNU Public License
*/
#include "task.h"

Task *task_list[MAX_NUM_TASKS];
static uint8_t num_tasks;

bool task_execute(void)
{
	bool result = false;
	uint8_t i;
	
	for (i=0; i<num_tasks; i++)
	{
		result |= task_list[i]->Execute();
	}
	return result;
}


Task::Task(void)
{
	if (num_tasks < MAX_NUM_TASKS)
	{
		task_list[num_tasks++] = this;
	}
}
