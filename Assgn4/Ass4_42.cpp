#include <iostream>
#include <string>
#include <sstream>
#include <bits/stdc++.h>
#include <unistd.h>
using namespace std;

#define M 80
#define QUANTUM 1
#define N 10

struct threads
{
	pthread_t tid;
	int id;
	// int status;
	char worker;
};

int BUFFER[M];
int pos = 0;
pthread_t P_threads[N];
pthread_t C_threads[N];
pthread_t scheduler;
pthread_t reporter;
vector<threads> ready_queue;
int num_p, num_c;
vector<int> status(N);

void catcher(int signum)
{
    if (signum == SIGUSR1)
    {
        sigset_t myset;
        sigemptyset(&myset);

        printf("Suspending thread\n");

        sigsuspend(&myset);
    }
    else
    {
        printf("Waking thread\n");

        return;
    }
}



void* producer(void *param)
{
    signal(SIGUSR1, catcher);
    signal(SIGUSR2, catcher);
    
    int i = 0;
    while(i < 1000)
    {
        if (pos < M-1)
        {
            pos++;
            BUFFER[pos] = rand() % 1000;
            i+=1;
        }
    }

    for(int i = 0; i<ready_queue.size();i++)
    {
    	if (ready_queue[i].tid == pthread_self())
    	{
    		printf("Removing %c%d\n", ready_queue[i].worker, ready_queue[i].id);
    	    ready_queue.erase(ready_queue.begin() + i);
    	    break;
    	}
    }
    pthread_exit(0);
}

void* consumer(void *param)
{
    signal(SIGUSR1, catcher);
    signal(SIGUSR2, catcher);
    
    while(1)
    {
        if (pos > 0)
            pos--;
    }

    // vector<pthread_t>::iterator it;
    // int i = 0;
    // for (it = ready_queue.begin(); it != ready_queue.end(); it++, i++)
    // {
    //     if (ready_queue[i] == pthread_self())
    //     {
    //         ready_queue.erase(ready_queue.begin() + i);
    //         break;
    //     }
    // }

    for(int i = 0; i< ready_queue.size();i++)
    {
    	if (ready_queue[i].tid == pthread_self())
    	{
    		printf("Removing %c%d\n", ready_queue[i].worker,ready_queue[i].id);
    	    ready_queue.erase(ready_queue.begin() + i);
    	    break;
    	}
    }
    pthread_exit(0);
}

void* report(void *param)
{
	cout<<"ID:\t";
	for(int i =0;i<ready_queue.size();i++)
	{
		printf("%c%d\t", ready_queue[i].worker, ready_queue[i].id);
	}
	printf("\n");
	cout<<"Status:\t";
	for(int i = 0; i<ready_queue.size();i++)
	{
		if(ready_queue[i].worker=='c')
			printf("%d\t", status[ready_queue[i].id+num_p]);
		// printf("Status:  %d\t", ready_queue[i].status);
		else
			printf("%d\t", status[ready_queue[i].id]);

	}
	printf("\n");
	pthread_exit(0);

}

void* schedule(void* param)
{
    int i;
    for (i = 0; i < num_p; i++)
    {
    	printf("Inside schedule. killing %d producer\n", i);
        pthread_kill(P_threads[i], SIGUSR1);
    }

    for (i = 0; i < num_c; i++)
    {
    	printf("Inside schedule. killing %d consumer\n", i);
        pthread_kill(C_threads[i], SIGUSR1);
    }
    sleep(1);

    for (i = 0; i < num_p; i++)
    {
    	threads thread;
    	thread.tid = P_threads[i];
    	thread.id = i;
    	// thread.status = 0;
    	thread.worker = 'p';
        ready_queue.push_back(thread);
    }

    for (i = 0; i < num_c; i++)
    {
		threads thread;
		thread.tid = C_threads[i];
		thread.id = i;
		// thread.status = 0;
		thread.worker = 'c';
	    ready_queue.push_back(thread);
    }

    while(1)
    {
        if (!ready_queue.size())
            break;
        pthread_t top = ready_queue[0].tid;
        pthread_kill(top, SIGUSR2);
        printf("\nKilled %c%d\n",ready_queue[0].worker,ready_queue[0].id);
        if(ready_queue[0].worker=='p')
        {
        	status[ready_queue[0].id] = 0;
        }
        else
        {
        	status[ready_queue[0].id+num_p] = 0;
        }
        // ready_queue[0].status = 0;
        if(1)
        {
	        pthread_t reporter;
	        pthread_attr_t attr_R;
	        pthread_attr_init(&attr_R);
	        pthread_create(&reporter,&attr_R,report,NULL); 
	        pthread_join(reporter,NULL);
	    }
        threads thread = ready_queue[0];
        ready_queue.erase(ready_queue.begin());
        ready_queue.push_back(thread);
        sleep(QUANTUM);
        pthread_kill(top, SIGUSR1);
        printf("Activated %c%d\n",ready_queue[0].worker,ready_queue[0].id);
        // ready_queue[0].status = 1;
        if(ready_queue[0].worker=='p')
        {
        	status[ready_queue[0].id] = 1;
        }
        else
        {
        	status[ready_queue[0].id+num_p] = 1;
        }
        if(1)
        {
	        pthread_t reporter;
	        pthread_attr_t attr_R;
	        pthread_attr_init(&attr_R);
	        pthread_create(&reporter,&attr_R,report,NULL); 
	        pthread_join(reporter,NULL);
	    }
        
    }

}




int main()
{
    srand(time(0));
    
    num_p = rand() % N;
    num_c = N - num_p;
    cout<< "value of p: "<<num_p<<" value of C:"<<num_c<<endl;
    

    for(int i = 0; i < num_p; i++)
    {
        pthread_attr_t attr_P;
        pthread_attr_init(&attr_P);
        pthread_create(&P_threads[i],&attr_P,producer,NULL); 
    }
    for(int i = 0; i < num_c ; i++)
    {

        pthread_attr_t attr_C;
        pthread_attr_init(&attr_C);
        pthread_create(&C_threads[i],&attr_C,consumer,NULL);          
    }
    pthread_create(&scheduler, NULL, schedule, NULL);
    // pthread_create(&reporter, NULL, report, NULL);

    for(int i =0; i<num_p; i++)
    {
        pthread_join(P_threads[i],NULL);
    }
    for(int i =0; i<num_c; i++)
    {
        pthread_join(C_threads[i],NULL);
    }

}