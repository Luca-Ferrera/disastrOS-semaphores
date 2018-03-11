#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"
#include "disastrOS_semaphore.h"

#define PRODUCERS_NUM 1
#define CONSUMERS_NUM 1
#define EMPTY_SEM_ID 0
#define FILL_SEM_ID 1
#define PRODUCERS_SEM_ID 2
#define CONSUMERS_SEM_ID 3
#define BUFFER_SIZE 128

int transactions[BUFFER_SIZE];  // circular buffer
int read_index;     // index of the next slot containing information to be read
int write_index;    // index of the next available slot for writing
int empty_sem;
int fill_sem;
int consumers_sem;
int producers_sem;
int deposit;

//TODO: should we declare semaphores globaly?

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

/** Producer **/
// TODO: Why does the func return a void* ?
void* producerJob(void* arg) {
    int ret = disastrOS_openSemaphore(PRODUCERS_SEM_ID, 0);
    return;
    while (1) {
        // produce the item
        int currentTransaction = 1;

        int ret = disastrOS_semWait(empty_sem);
        //TODO: manage error

        // write the item and update write_index accordingly
        transactions[write_index] = currentTransaction;
        write_index = (write_index + 1) % BUFFER_SIZE;

        //ret = disastrOS_semPost(&fill_sem)
        //TODO: manage error
    }
}

/** Consumer **/
// TODO: Why does the func return a void* ?
void* consumerJob(void* arg) {
    int ret = disastrOS_openSemaphore(CONSUMERS_SEM_ID, 0);
    return;
    while (1) {
        int ret = disastrOS_semWait(fill_sem);
        //TODO: manage error

        ret = disastrOS_semWait(producers_sem);
        //TODO: manage error

        // get the item and update read_index accordingly
        int lastTransaction = transactions[read_index];
        read_index = (read_index + 1) % BUFFER_SIZE;

        //ret = disastroOS_semPost(&stop_producer);
        //TODO: manage error

        //ret = disastrOS_semPost(&empty_sem);
        //TODO: manage error

        // consume the item
        deposit += lastTransaction;
        if (read_index % 10 == 0) {
            printf("After the last 10 transactions balance is now %d.\n", deposit);
        }
    }
}

void childFunction(void* args){
  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");

  if (((int*) args)[0] == PRODUCERS_SEM_ID)
    producerJob(PRODUCERS_SEM_ID);
  else if (((int*) args)[0] == CONSUMERS_SEM_ID)
    consumerJob(CONSUMERS_SEM_ID);

  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);
  
  int i, ret;
  // Creating empty and fill semaphores
  empty_sem = disastrOS_openSemaphore(EMPTY_SEM_ID, DSOS_CREATE | DSOS_EXCL ,BUFFER_SIZE);
  // Reopening the same just for testing purposes
  empty_sem = disastrOS_openSemaphore(EMPTY_SEM_ID, DSOS_CREATE);
  fill_sem = disastrOS_openSemaphore(FILL_SEM_ID, DSOS_CREATE | DSOS_EXCL, 0);
 
  // Creating producers/consumers mutex semaphores
  producers_sem = disastrOS_openSemaphore(PRODUCERS_SEM_ID, DSOS_CREATE | DSOS_EXCL, 1);
  consumers_sem = disastrOS_openSemaphore(CONSUMERS_SEM_ID, DSOS_CREATE | DSOS_EXCL, 1);

  disastrOS_printStatus();
  printf("Shutdown!\n");
  disastrOS_shutdown();

  printf("[+] Creating %d producers and %d consumers\n", PRODUCERS_NUM, CONSUMERS_NUM);

  int alive_children=0;
  int job_type;
  for (int i = 0, job_type = PRODUCERS_SEM_ID; i<PRODUCERS_NUM; ++i) {
    disastrOS_spawn(childFunction, &job_type);
    alive_children++;
  }
  for (int i = 0, job_type = CONSUMERS_SEM_ID; i<CONSUMERS_NUM; ++i) {
    disastrOS_spawn(childFunction, &job_type);
    alive_children++;
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, alive_children);
    --alive_children;
  }
  printf("shutdown!");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  write_index = 0;
  read_index = 0;
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer is: %p", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
