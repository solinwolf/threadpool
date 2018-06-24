#include <stdio.h>
#include <pthread.h>

#include <unistd.h>
#include "threadpool.h"
#include <stdio.h>
extern threadpool_t *pool;

void *myhandler(void* arg)
{
	printf("job_routing...\n");
	return (void*)0;
}
int main(int argc,char** argv)
{
	int threadnum = 4;
	int count = 0;
	threadpool_init(threadnum);
	printf("main thread...\n");
	for(count=0;count<threadnum;count++)
	{
		threadpool_add_job(myhandler, NULL);
	}
	sleep(10);
	threadpool_destroy(pool);
	return 0;
}