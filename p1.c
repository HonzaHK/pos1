//FIT VUTBR - POS - project 1
//JAN KUBIS / xkubis13
#define _POSIX_C_SOURCE 199506L
//#define _REENTRANT //gcc yells redefinition

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/time.h>
#include <pthread.h>

typedef struct { //comand line arguments
	int threadCount;
	int loopCount;
} clargs_t;
clargs_t clargs;

pthread_mutex_t ticketGeneratorMutex; //mutual exclusion during ticket number assignment
int ticketsAssignedCount = 0; //shared variable protected by ticketGeneratorMutex

typedef struct { //critical section variables
    pthread_cond_t cond;
    pthread_mutex_t mutex;
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
	pthread_cond_init(&csLock.cond,NULL);
	 
	pthread_t threads[clargs.threadCount];
	int t_ids[clargs.threadCount];

	for(int i=0; i<clargs.threadCount; i++){
		t_ids[i] = i+1; //threads are 1-N (if id==0, rand_r seed will be always 0)
		if(pthread_create(&threads[i], NULL, &threadFunc, &t_ids[i])) {
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}
	}

	for(int i=0; i<clargs.threadCount; i++){
		pthread_join(threads[i], NULL);
	}

	pthread_mutex_destroy(&ticketGeneratorMutex);
	pthread_mutex_destroy(&csLock.mutex);
	pthread_cond_destroy(&csLock.cond);

	return 0;
}