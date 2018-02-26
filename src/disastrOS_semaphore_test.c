#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"
#include "disastrOS_semaphore.h"

#define THREADS_NUM 1
#define SEM_ID 1
#define BUFFER_SIZE 128

int transactions[BUFFER_SIZE];  // circular buffer
int read_index;     // index of the next slot containing information to be read
int write_index;    // index of the next available slot for writing

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
void* producerJob(void* arg) {
    while (1) {
        // produce the item
        int currentTransaction = 1;

        int ret = disastrOS_semWait(&empty_sem);
        //TODO: manage error

        // write the item and update write_index accordingly
        transactions[write_index] = currentTransaction;
        write_index = (write_index + 1) % BUFFER_SIZE;

        //ret = disastrOS_semPost(&fill_sem)
        //TODO: manage error
    }
}

/** Consumer **/
void* consumerJob(void* arg) {
    while (1) {
        int ret = disastrOS_semWait(&fill_sem);
        //TODO: manage error

        ret = disastrOS_semWait(&stop_producer);
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

  int sem_id = disastrOS_openSemaphore(SEM_ID, 10);
  printf("sem_id=%d\n", sem_id);
  
  for (int i=0; i<(disastrOS_getpid()+1); ++i){
    printf("PID: %d, iterate %d\n", disastrOS_getpid(), i);
    disastrOS_sleep((20-disastrOS_getpid())*5);
  }
  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);
  

  printf("I feel like to spawn %d nice threads\n", THREADS_NUM);
  int alive_children=0;
  for (int i=0; i<THREADS_NUM; ++i) {
    disastrOS_spawn(childFunction, 0);
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
