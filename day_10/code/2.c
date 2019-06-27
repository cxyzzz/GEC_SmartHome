#include<stdio.h>
#include <pthread.h>
void * fun(void * arg)
{
	int i;
	for(i=0;i<40;i++)
	{
		printf("fun: 222\n");
		usleep(100*1000);
	}
}



int main()
{
	int i;
	pthread_t pid;
	pthread_create(&pid,NULL,fun,NULL);
	//fun();
	
	for(i=0;i<20;i++)
	{
		printf("main : 111\n");
		usleep(100*1000);
	}
	
	pthread_join(pid,NULL);
	return 0;
}