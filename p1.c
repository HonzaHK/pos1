//FIT VUTBR - POS - project 1
//JAN KUBIS / xkubis13

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/time.h>
#include <pthread.h>

typedef struct {
	int threadCount;
	int loopCount;
} clargs_t;
clargs_t clargs;

pthread_mutex_t ticketGeneratorMutex;
int ticketsAssignedCount = 0;

typedef struct {
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    unsigned long queue_head, queue_tail;
    int currentTicket;
} ticket_lock_t;
ticket_lock_t csLock;


void printHelp(){
	printf("FIT VUT - POS - project 1  |  Jan Kubis\n");
	printf("Usage: ./p1 N M\n");
	printf("(N - number of threads; M - number of loops");
}

int parseArgs(int argc, char* argv[], clargs_t* clargs){

	if(argc!=3){ 
		return 1;
	}
	else if(!isdigit(*argv[1])){
		return 1;
	}
	else if(!isdigit(*argv[2])){
		return 1;
	}

	clargs->threadCount=strtol(argv[1],NULL,10);
	clargs->loopCount=strtol(argv[2],NULL,10);

	return 0;
}

void await(int aenter){

	pthread_mutex_lock(&csLock.mutex);
	while (aenter != csLock.currentTicket){
		pthread_cond_wait(&csLock.cond, &csLock.mutex);
	}
	pthread_mutex_unlock(&csLock.mutex);
}

void advance(){
	pthread_mutex_lock(&csLock.mutex);
	csLock.currentTicket++;
	pthread_cond_broadcast(&csLock.cond);
	pthread_mutex_unlock(&csLock.mutex);
}

int getticket(){
	pthread_mutex_lock(&ticketGeneratorMutex);
	int nth = ticketsAssignedCount;
	ticketsAssignedCount++;
	pthread_mutex_unlock(&ticketGeneratorMutex);
	return nth;
}

void myNanoSleep(int threadId){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned int time_usec = 1000000 * tv.tv_sec + tv.tv_usec;
	
	unsigned int seed = time_usec * threadId;
	struct timespec req, rem;	
	req.tv_sec = 0;
	req.tv_nsec = rand_r(&seed) % 500000000;
	
	//printf("id: %d  |  ns: %d\n",threadId,req.tv_nsec );
	nanosleep(&req , &rem);
}

void *threadFunc(void *id_voidptr){
	int threadId = *((int *)id_voidptr);
	//printf("t_id:%d (%ld)\n", id,pthread_self());


	int ticket;
	while ((ticket = getticket()) < clargs.loopCount) { /* Přidělení lístku */
	   	myNanoSleep(threadId);
	    await(ticket);              /* Vstup do KS */
	    printf("%d\t(%d)\n", ticket, threadId); /* fflush(stdout); */
	    advance();              /* Výstup z KS */
	   	myNanoSleep(threadId);
	    /* Náhodné čekání v intervalu <0,0 s, 0,5 s> */
	}

	return NULL;
}



int main(int argc, char* argv[]){
	
	int errno = 0;	
	if((errno=parseArgs(argc,argv,&clargs))!=0){
		printHelp();
		return errno;
	}

	pthread_mutex_init(&ticketGeneratorMutex,NULL); 
	pthread_mutex_init(&csLock.mutex,NULL); 
	pthread_t threads[clargs.threadCount];

	for(int i=0; i<clargs.threadCount; i++){
		int *t_id = malloc(sizeof(int));
		*t_id = i;
		if(pthread_create(&threads[i], NULL, &threadFunc, t_id)) {
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}
	}

	for(int i=0; i<clargs.threadCount; i++){
    	pthread_join(threads[i], NULL);
	}

	return 0;
}