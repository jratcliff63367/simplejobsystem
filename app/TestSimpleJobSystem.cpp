#include <stdio.h>

#define TEST_FOR_MEMORY_LEAKS 0 // set to 1, on Windows only, to enable memory leak checking on application exit
#if TEST_FOR_MEMORY_LEAKS
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#else
#include <stdlib.h>
#endif

#include <string.h>
#include <stdint.h>

#define ENABLE_SIMPLE_JOB_SYSTEM_IMPLEMENTATION 1
#include "SimpleJobSystem.h"
#include "ScopedTime.h"

#include <thread>
#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma warning(disable:4100 4996)
#include <conio.h>
#endif

// Evaluates if this is true or false, returns true if it
// could be evaluated. Stores the result into 'value'
bool getTrueFalse(const char *option,bool &value)
{
	bool ret = false;

	if ( strcmp(option,"t") == 0 ||
		 strcmp(option,"true") == 0 ||
		 strcmp(option,"1") == 0 )
	{
		ret = true;
		value = true;
	}
	else if ( strcmp(option,"f") == 0 ||
		 strcmp(option,"false") == 0 ||
		 strcmp(option,"0") == 0 )
	{
		ret = true;
		value = false;
	}
	else
	{
		printf("Valid values are 'true' or 'false', 't' or 'f', or '1' or '0', only.\n");
	}

	return ret;
}

static const double TASK_TIME = 0.0001;

void doTask(void *ptr)
{
	Timer t;
	while ( t.peekElapsedSeconds() < TASK_TIME );
}

void performJobs(uint32_t jobCycleCount,uint32_t jobTaskCount,simplejobsystem::SimpleJobSystem *sjs)
{
	for (uint32_t i=0; i<jobCycleCount; i++)
	{
		for (uint32_t j=0; j<jobTaskCount; j++)
		{
			if ( sjs )
			{
				sjs->addJob(nullptr,doTask);
			}
			else
			{
				doTask(nullptr);
			}
		}
		if ( sjs )
		{
			sjs->startJobs();
			sjs->waitForJobsToComplete();
		}
	}
}

int main(int argc,const char **argv)
{
#if TEST_FOR_MEMORY_LEAKS
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	if ( argc < 2 )
	{
		printf("Usage: TestSimpleJobSystem (options)\n");
		printf("\n");
		printf("-jc <count>         : Number of cycles to run the simple job system. Default 100.\n");
		printf("-jt <count>         : Number of tasks to run each cyce. Default 100.\n");
	}
	else
	{
		uint32_t jobCycleCount = 100;
		uint32_t jobTaskCount = 100;
		{
			for (int i=1; i<argc; i+=2)
			{
				const char *option = argv[i];
				const char *value = argv[i+1];
				if ( strcmp(option,"-jc") == 0 )
				{
					jobCycleCount = atoi(value);
					printf("JobCycleCount=%d\n", jobCycleCount);
				}
				else if ( strcmp(option,"-jt") == 0 )
				{
					jobTaskCount = atoi(value);
					printf("JobTaskCount=%d\n", jobTaskCount);
				}

			}
			simplejobsystem::SimpleJobSystem *sjs = simplejobsystem::SimpleJobSystem::create(8,nullptr);
			{
				ScopedTime st("non-async");
				performJobs(jobCycleCount,jobTaskCount,nullptr);
			}
			{
				ScopedTime st("async");
				performJobs(jobCycleCount,jobTaskCount,sjs);
			}
			sjs->release();
		}
	}
	return 0;
}
