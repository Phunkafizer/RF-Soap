#ifdef __cplusplus

//this is needed for pure abstract classes

extern "C"
{
	#include <stdlib.h>
	
	void __cxa_pure_virtual()
	{
	}
}

void *operator new(size_t size)
{
	return malloc(size);
}

void *operator new[](size_t size)
{
	return malloc(size);
}

void operator delete(void* ptr)
{
	free(ptr);
}

void operator delete[](void* ptr)
{
	free(ptr);
}

#endif