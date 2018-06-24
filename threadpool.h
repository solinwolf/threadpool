#ifndef _THREADPOOL_H__
#define _THREADPOOL_H__
#include <pthread.h>
//定义两个常量，表示线程池的销毁与否
typedef enum
{
	on = 1,
	off = 0
}threadpoolflag;
//定义线程处理函数类型
typedef void* (*thread_handler_t)(void*);
//定义任务结构，每个结构由处理函数、处理函数参数和指向下一个任务结构的指针构成
typedef struct job_tag
{
	thread_handler_t handler;
	void *args;
	struct job_tag* next;
	
}job_t;


//定义线程池结构
typedef struct threadpool_tag
{
	int thread_max_num;  //线程池最大的线程数
	int job_cur_num;     //当前线程池的任务数
	pthread_mutex_t lock; //线程池互斥量
	pthread_cond_t cond;  //线程池条件参数
	job_t* head;          //指向任务链表的指针
	pthread_t* pthreads;  //所有线程的线程ID组成一个单向链表
	int flag;             //线程池的状态，flag=on表示线程池不需要销毁，flag=off表示线程池将要销毁
}threadpool_t;        
//线程处理函数
void *thread_handler(void *arg);
//线程池初始化，负责分配线程池结构并初始化，创建所有线程
void threadpool_init(int threadnum);
//创建任务结构，并将任务加入到线程池中
int threadpool_add_job(void *(*process) (void *arg),void *arg);
//销毁线程池,释放线程和线程池所占资源
int threadpool_destroy(threadpool_t *pool);

#endif