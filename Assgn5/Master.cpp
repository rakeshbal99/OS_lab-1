
#include <iostream>
#include <string>
#include <sstream>
#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <signal.h>
#include <semaphore.h>
#include <sys/msg.h>
#include <unordered_map>
#include <typeinfo>

using namespace std;

// struct page_entry{
// 	int page;
// 	int frame;
// 	int valid;
// 	int use;
// };

// struct main_mem_frame{
// 	int frame;
// 	int valid;
// 	int use;
// };

struct page_entry{
	int page;
	int frame;
	int use;
	int valid;
};

struct main_mem_frame{
	int frame;
	int free;
	int use;
};

pid_t sched_pid, mmu_pid;

void catcher(int signum)
{

    kill(sched_pid, SIGKILL);
    kill(mmu_pid, SIGKILL);
    cout<<"MASTER TERMINATES";
    exit(EXIT_SUCCESS);
}


int main()
{
	signal(SIGUSR1, catcher);
	int k, m, f, s;

	cout<<"Enter the number of processes : ";
	cin>>k;
	cout<<"Enter the number maximum number of pages required per process : ";
	cin>>m;
	cout<<"Enter the total number of main frames in memory : ";
	cin>>f;
	cout<<"Enter the size of TLB : ";
	cin>>s;

	pid_t process_pid[k];

	// std::unordered_map<int, int> *TLB_ref;
	// std::unordered_map<int, int> TLB1;

	key_t key_1 = ftok("SM1",65); 
	int shmid_1 = shmget(key_1, k * m * sizeof(page_entry), 0666|IPC_CREAT); 
	  
	key_t key_2 = ftok("SM2",65); 
	int shmid_2 = shmget(key_2, f * sizeof(main_mem_frame), 0666|IPC_CREAT); 

    key_t key_3 = ftok("MQ1", 65);
    int msgid_1 = msgget(key_3, 0666 | IPC_CREAT);

    key_t key_4 = ftok("MQ2", 65);
    int msgid_2 = msgget(key_4, 0666 | IPC_CREAT);

    key_t key_5 = ftok("MQ3", 65);
    int msgid_3 = msgget(key_5, 0666 | IPC_CREAT);

    page_entry *pge = (page_entry*) shmat(shmid_1,(void*)0,0);
    main_mem_frame *mmf = (main_mem_frame*) shmat(shmid_2,(void*)0,0);

	char key_1_str[20], key_2_str[20], key_3_str[20], key_4_str[20], key_5_str[20];
	sprintf(key_1_str,"%d", key_1);
	sprintf(key_2_str,"%d", key_2);
	sprintf(key_3_str,"%d", key_3);
	sprintf(key_4_str,"%d", key_4);
	sprintf(key_5_str,"%d", key_5);

	for (int i = 0; i < k * m; i++)
	{
		pge[i].page = i;
		pge[i].frame = -1;
		pge[i].use = 0;
		pge[i].valid = 0;
	}	

	for (int i = 0; i < f; i++)
	{
		mmf[i].frame = i;
		mmf[i].free = 1;
		mmf[i].use = 0;
	}

	if( (sched_pid = fork()) == 0)
	{
        execlp("./scheduler", "./scheduler", key_3_str, key_4_str, (char *) NULL);
        printf("Failed to start scheduler \n");
        exit(EXIT_FAILURE);
    }

    if((mmu_pid =fork())== 0)
    {
    	char s_str[10], m_str[10], k_str[10], f_str[10];
    	sprintf(s_str, "%d", s);
    	sprintf(m_str, "%d", m);
    	sprintf(k_str, "%d", k);
    	sprintf(f_str, "%d", f);
    	execlp("./mmu", "./mmu", key_4_str, key_5_str, key_1_str, key_2_str, s_str, m_str, k_str, f_str, (char *) NULL); // send others
    	printf("Failed to start mmu \n");
    	exit(EXIT_FAILURE);
    }

    for( int i =0; i<k; i++)
    {
    	int m_i = rand()%m + 1;
    	int p_i = 2*m_i + rand()%(10*m_i+1);
    	char m_str[10], p_i_str[10], i_str[10];
    	sprintf(m_str,"%d", m);
    	sprintf(p_i_str,"%d", p_i);
    	sprintf(i_str,"%d", i);
    	for(int j =i*m+m_i; j<i*m+m; j++)
    	{
    		pge[j].valid = 1;
    		pge[j].frame = -2;
    	}
	    if((process_pid[i] = fork()) == 0)
	    {
	    	execlp("./process", "./process", key_3_str, key_5_str, p_i_str, m_str, i_str, (char *) NULL); // send others
	    	printf("Failed to start process \n");
	    	exit(EXIT_FAILURE);
	    }
	    usleep(250000);
	}
    while(1)
    {
    	sleep(1);
    }
}
