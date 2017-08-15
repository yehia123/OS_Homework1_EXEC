#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

/*max time (in seconds)*/
#define MAX_SLEEP_TIME 3

/* number of potential students */
#define NUM_OF_STUDENTS 4

/* number of times each student may receive help (allowance/meter) */
#define NUM_OF_HELPS 2

/* number of available seats */
#define NUM_OF_SEATS 2

/* structure for passing data to threads */
typedef struct
{
	int id;
	int seed;
	int meter; // remaining "helps"
}
parameters;

/* mutex declarations, a global variable */
pthread_mutex_t mutex_lock; /* protect the global variable waiting_student */

/* semaphore declarations, global variables */
sem_t students_sem; /* ta waits for a student to show up, student notifies ta his/her arrival:*/
sem_t ta_sem; /* student waits for ta to help, ta notifies student he/she is ready to help:*/

/* the number of waiting students, a global variable */
int waiting_students;

int seat[NUM_OF_SEATS]; // last student to sit in each seat; for identification, not synchronization, purposes

/* threads run these functions */
void *doTA();
void *doStudent();

int main(int argc, char *argv[]){
	
	printf("CS149 Sleeping TA from Yehia Qtaish\n");

	/*Create student semaphor and initialize it to 0*/
	sem_init(&students_sem, 0, 0);

	/*Create the TA \semaphor and initialize it to 1=ready*/
	sem_init(&ta_sem, 0, 1);

	/*Create the mutex lock*/
	pthread_mutex_init(&mutex_lock, NULL);

	// allocate shared pthread attribute structure
	pthread_attr_t attr;

	// initialize pthread attributes
	pthread_attr_init(&attr);

	// declare worker thread structures
	pthread_t tid[NUM_OF_STUDENTS];
	pthread_t tid0; // TA thread id

	// save their parameter structure pointers to free when each worker exits
	parameters data[NUM_OF_STUDENTS];

	waiting_students = 0; // not critical (yet) since no threads started

	pthread_create(&tid0,&attr,doTA,NULL); // start TA thread

	int k;
	// create student worker threads
	for(k=0; k<NUM_OF_STUDENTS; k++)
	{
		data[k].id = k+1;
		pthread_create(&tid[k],&attr,doStudent,&data[k]);
	}

	// wait for student threads to complete and free their param
	for (k=1; k<=NUM_OF_STUDENTS; k++)
	{
		pthread_join(tid[k],NULL); // start student k thread
	}

	pthread_cancel(tid0);
}

void *doTA(void *data)
{
	int seed = -1;

	while (1)
	{
		sem_wait(&students_sem);
		while (waiting_students > 0)
		{
			// process seated students post notifies TA
			sem_post(&ta_sem);
		pthread_mutex_lock(&mutex_lock);
			int sid = seat[0];
			for (int i=1; i<waiting_students; i++) seat[i-1]=seat[i]; // rotate seated students
			waiting_students--; //decrements # of students after helping one that is waiting
		pthread_mutex_unlock(&mutex_lock);

			int w = (rand_r(&seed) % MAX_SLEEP_TIME) + 1;
			printf("Helping student %d for %d seconds, # of waiting students = %d\n", sid, w, waiting_students);
			sleep(w);
		}
	}
}

void *doStudent(void *param)
{
	parameters *data = (parameters *) param;
	data->seed = data->id; // copy id into (changing) seed
	data->meter = NUM_OF_HELPS; // meter counts down to track if students are currently waiting

	while (data->meter > 0)
	{
		// students start each iteration by programming, then by seeking help
		int w = (rand_r(&data->seed) % MAX_SLEEP_TIME) + 1;
		printf("\tStudent %d programming for %d seconds\n", data->id, w);

    
		// Decide, mutex locks to protect global varaible
		pthread_mutex_lock(&mutex_lock);
		if (waiting_students == 0) // "If the TA is available, [student] obtains help."
		{
			sem_wait(&ta_sem); // obtain TA's help, lock TA
			printf("Student %d receiving help\n", data->id);
			data->meter--;
			sem_post(&students_sem);
		}
		else if (waiting_students < NUM_OF_SEATS) // a chair is available to wait
		{
			seat[waiting_students++] = data->id;
			printf("\t\tStudent %d takes a seat, # of waiting students = %d\n", data->id, waiting_students);
		}
		else // resume programming
		{
			printf("\t\t\tStudent %d will try later\n", data->id);
		}
		pthread_mutex_unlock(&mutex_lock);

	} // data_meter

	pthread_exit(0);
}
