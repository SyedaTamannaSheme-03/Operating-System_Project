#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX 1000000


int new_visited[MAX]; 
int priority_no[MAX];
int arrival_time[MAX];

int help[MAX]; 
int student_ids[MAX]; 
int tutor_ids[MAX]; 

sem_t student;
sem_t coordinator;
sem_t tutor[MAX];
sem_t mutexLock;

int requesting_time = 0;
int students_done = 0; 
int help_no = 0;
int total_chair = 0;
int occupied_chairs=0;
int total_student = 0;
int total_tutor = 0;


void *student_thread(void *student_id)
{ 
    int s_id=*(int*)student_id; 
    
    while(1)
	{ 
        if(help[s_id-1] == help_no) 
		{ 
            
			sem_wait(&mutexLock);
            students_done++; 
			sem_post(&mutexLock);
   
            printf("\n\n\t\t\tstudent %d terminates\n\n",s_id);
			if(students_done == total_student)
			{
				printf("\n\t\t  All Students Have Recieved Help\n");
			}			 
            sem_post(&student);
            pthread_exit(NULL); 
        }
        
        
        
		sem_wait(&mutexLock); 
        if(occupied_chairs == total_chair)
		{
			sem_post(&mutexLock); 
            continue;
        }
        
        occupied_chairs++; 
        requesting_time++; 
        new_visited[s_id-1]=requesting_time;
        
        printf("\nStudent Thread: Student %d takes a Seat.\nStudent Thread: Empty Chairs = %d\n",s_id,total_chair-occupied_chairs);
  		sem_post(&mutexLock); 
        
        sem_post(&student); 
        
		sem_wait(&tutor[s_id-1]); 

        printf("\nStudent Thread: Student %d Received Help.\n",s_id);
               
		sem_wait(&mutexLock);
        help[s_id-1]++;
		printf("\nStudent Thread: Student %d's Priority now is %d\n",s_id, help[s_id-1]);
		sem_post(&mutexLock);
    }
}

void *coordinator_thread()
{
    while(1)
	{
        if(students_done==total_student)
		{
            for(int i=0;i<total_tutor;i++)
			{
	        sem_post(&coordinator);
            }
            
			printf("\n\n\t\t\tcoordinator terminates\n\n");
            pthread_exit(NULL);
            
        }
               
        sem_wait(&student); 

       
		sem_wait(&mutexLock);
        for(int i=0;i<total_student;i++)
		{
            if(new_visited[i]>-1)
			{
                priority_no[i] = help[i];
                arrival_time[i] = new_visited[i];
                
                printf("\nCoordinator Thread: Student %d with Priority %d in the queue.\n",student_ids[i],help[i]);
                new_visited[i]=-1;
				
                sem_post(&coordinator);
            }
        }
		sem_post(&mutexLock);
    }
}

void *tutor_thread(void *tutor_id)
{
       int t_id=*(int*)tutor_id;
    
    while(1)
	{
        if(students_done==total_student)
		{ 
        sem_wait(&mutexLock);
            printf("\n\n\t\t\ttutor %d terminates\n\n",t_id);
            
            sem_post(&mutexLock);
            
            pthread_exit(NULL); 
        }
        
        int max_request=total_student*help_no+1;
		int max_priority = help_no-1;
        int s_id=-1; 
		
        sem_wait(&coordinator);
        
		sem_wait(&mutexLock);
       for(int i=0;i<total_student;i++)
		{
			if(priority_no[i]>-1 && priority_no[i]<= max_priority)
			{
				if(arrival_time[i]<max_request)
				{
					max_priority=priority_no[i]; 
					max_request=arrival_time[i]; 
					s_id=student_ids[i];
				}
			}
			         
        }
        
        if(s_id==-1) 
		{
			sem_post(&mutexLock);
            continue;
        }
        
		priority_no[s_id-1] = -1;
		arrival_time[s_id-1] = -1;
        
        occupied_chairs--;
		sem_post(&mutexLock);
        
        
		sem_wait(&mutexLock);
        printf("\nTutor Thread: Student %d is tutored by Tutor %d\n",s_id,t_id);
		sem_post(&mutexLock);

		sem_post(&tutor[s_id-1]);
        
    }
}






int main()
 {
	printf("---------------Seeking Tutor Problem---------------\n");
	printf("Enter the Following values: \n");
	
	printf("Total number of students: ");
	scanf("%d", &total_student);
	printf("Total number of Tutors: ");
	scanf("%d", &total_tutor);
	printf("Total number of Chairs: ");
	scanf("%d", &total_chair);
	printf("Maximum number of help a student can get: ");
	scanf("%d", &help_no);
	
    
    for(int i=0;i<total_student;i++)
	{
        new_visited[i]=-1; 
        priority_no[i] = -1;
        arrival_time[i] = -1;
        help[i]=0; 
    }

    sem_init(&student,0,0);
    sem_init(&coordinator,0,0);
	sem_init(&mutexLock,0,1);
	for(int i=0;i<total_student;i++)
	{
        sem_init(&tutor[i],0,0);
	}
    
    pthread_t students[total_student];
    pthread_t tutors[total_tutor];
    pthread_t coordinator;
    

    for(int i = 0; i < total_student; i++)
    {
        student_ids[i] = i + 1; 
        pthread_create(&students[i], NULL, student_thread, (void*) &student_ids[i]);
        
    }
    

    for(int i = 0; i < total_tutor; i++)
    {
        tutor_ids[i] = i + 1;
		pthread_create(&tutors[i], NULL, tutor_thread, (void*) &tutor_ids[i]);

    }


	pthread_create(&coordinator,NULL,coordinator_thread,NULL);

    
    for(int i =0; i < total_student; i++)
    {
        pthread_join(students[i],NULL);
    }
	
    for(int i =0; i < total_tutor; i++)
    {
        pthread_join(tutors[i],NULL);
    }
	
	pthread_join(coordinator, NULL);
    
 }




