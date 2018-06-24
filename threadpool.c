#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include "threadpool.h"
threadpool_t *pool = NULL;



//线程处理函数,当线程创建成功，线程开始执行这个函数
//每个线程都会循环等待线程池是否有任务到来，如果没有任务，则一直等待，如果有任务到来，则从任务链表中取出任务进行执行
void *thread_handler(void *arg)
{
	printf ("starting thread 0x%x\n", pthread_self());
	
	while(1)
	{
		//等待任务到来，任务来了后退出循环，开始执行任务
		pthread_mutex_lock(&(pool->lock));
		while((pool->job_cur_num==0)&&(pool->flag==on))
		{
			pthread_cond_wait(&(pool->cond),&(pool->lock));
		}
		//销毁线程池标志位置位，退出线程
		if(pool->flag==off)
		{
            pthread_mutex_unlock(&(pool->lock)); 
            printf ("thread 0x%x will exit\n", pthread_self()); 
            pthread_exit(NULL);
		}
		//从任务链表中取出一个任务进行执行，并将任务数减一
		pool->job_cur_num--;
		job_t* job = pool->head;
		pool->head = job->next;
		pthread_mutex_unlock(&(pool->lock));
		//调用任务函数
		(*(job->handler))(job->args);
		//任务函数返回，说明任务执行完成，结束本次任务,线程继续查看是否有新任务
		free(job);
		job = NULL;
	}
	//应该不会到达
	pthread_exit(NULL);
}

//线程池初始化，负责分配线程池结构并初始化，创建所有线程
void threadpool_init(int threadnum)
{
	int count = 0;
	if(threadnum<=0)
		return ;
	//分配线程池结构
	if((pool=(threadpool_t*)malloc(sizeof(threadpool_t)))==NULL)
		return ;
	//初始化线程池结构变量
	pool->thread_max_num = threadnum;
	pool->job_cur_num = 0;
	pthread_mutex_init(&(pool->lock),NULL);
	pthread_cond_init(&(pool->cond),NULL);
	pool->head = NULL;
	pool->flag = on;
	//分配线程ID数组
	if((pool->pthreads=(pthread_t*)malloc(sizeof(pthread_t)*threadnum))==NULL)
		return;
	//创建线程池所有线程。线程创建后均开始执行thread_handler()函数
	for(count=0;count<threadnum;count++)
	{
		pthread_create(&(pool->pthreads[count]),NULL,thread_handler,NULL);
	}
}

//创建任务结构，并将任务加入到线程池中
int threadpool_add_job(void *(*process)(void *arg),void *arg)
{
	//分配任务结构
	job_t *job = (job_t*)malloc(sizeof(job_t));
	if(!process||!job)
		return -1;
	//用传进来的参数初始化任务结构变量
	job->handler = process;
	job->args = arg;
	//将任务加入任务队列，因为线程池是“先来先服务”类型，所以新任务加到队尾
	job->next = NULL;
	job_t *tmp = pool->head;
	pthread_mutex_lock(&(pool->lock)); 
	if(tmp)
	{
		while(tmp->next!=NULL)
			tmp = tmp->next;
		tmp->next = job;
	}
	//如果是第一次加任务，则直接加到线程池任务链表头
	else
	{
		pool->head = job;
	}
	pool->job_cur_num++;
    pthread_mutex_unlock(&(pool->lock)); 
    pthread_cond_signal(&(pool->cond)); 
	pool->flag = on;
	return 0;
}

//销毁线程池,释放线程和线程池所占资源
int threadpool_destroy(threadpool_t *pool)
{
    if (pool->flag==off) 
        return -1;/*防止两次调用*/ 
    pool->flag = off; 

    /*唤醒所有等待线程，线程池要销毁了*/ 
    pthread_cond_broadcast(&(pool->cond)); 

    /*阻塞等待线程退出，否则就成僵尸了*/ 
    int i; 
    for (i = 0; i < pool->thread_max_num; i++) 
        pthread_join(pool->pthreads[i], NULL); 
    free (pool->pthreads); 

    /*销毁等待队列*/ 
    job_t *head = NULL; 
    while (pool->head != NULL) 
    { 
        head = pool->head; 
        pool->head = pool->head->next; 
        free (head); 
    } 
    /*条件变量和互斥量也别忘了销毁*/ 
    pthread_mutex_destroy(&(pool->lock)); 
    pthread_cond_destroy(&(pool->cond)); 
     
    free (pool); 
    /*销毁后指针置空是个好习惯*/ 
    pool=NULL; 
    return 0;

}




