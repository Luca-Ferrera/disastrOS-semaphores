#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"
#include "disastrOS_semaphore.h"

#define ERROR_HANDLER(ret, msg) \
  do {\
    if (ret < 0) { \
      fprintf(stderr, "[!] %s\n", msg); \
      disastrOS_exit(1); \
    }\
  } while(0);

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

/** Producer **/
// TODO: Why does the func return a void* ?
void producerJob(void* arg) {
  printf("I'm a fucking prod\n");
    int ret = disastrOS_openSemaphore(PRODUCERS_SEM_ID, 0);
    while (1) {
        // produce the item
        int currentTransaction = 1;

        printf("[*] Waiting on %d\n", EMPTY_SEM_ID);
        int ret = disastrOS_semWait(empty_sem);
        //TODO: manage error
        disastrOS_printStatus();
        return;

        // write the item and update write_index accordingly
        transactions[write_index] = currentTransaction;
        write_index = (write_index + 1) % BUFFER_SIZE;

        ret = disastrOS_semPost(&fill_sem);
        //TODO: manage error
    }
}

/** Consumer **/
// TODO: Why does the func return a void* ?
void consumerJob(void* arg) {
    int ret = disastrOS_openSemaphore(CONSUMERS_SEM_ID, 0);
    while (1) {
        int ret = disastrOS_semWait(fill_sem);
        //TODO: manage error

        ret = disastrOS_semWait(producers_sem);
        //TODO: manage error
        return;

        // get the item and update read_index accordingly
        int lastTransaction = transactions[read_index];
        read_index = (read_index + 1) % BUFFER_SIZE;

        ret = disastroOS_semPost(&producers_sem);
        //TODO: manage error

        ret = disastrOS_semPost(&empty_sem);
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
  
  int i, ret;
  // Creating empty and fill semaphores
  empty_sem = disastrOS_openSemaphore(EMPTY_SEM_ID, DSOS_CREATE | DSOS_EXCL ,BUFFER_SIZE);
  ERROR_HANDLER(empty_sem, "Error opening empty_sem");
  // Reopening the same sempahore just for error testing purposes
  // empty_sem = disastrOS_openSemaphore(EMPTY_SEM_ID, DSOS_CREATE);
  // ERROR_HANDLER(empty_sem, "Error opening empty_sem");
  fill_sem = disastrOS_openSemaphore(FILL_SEM_ID, DSOS_CREATE | DSOS_EXCL, 0);
  ERROR_HANDLER(empty_sem, "Error opening fill_sem");
 
  // Creating producers/consumers mutex semaphores
  producers_sem = disastrOS_openSemaphore(PRODUCERS_SEM_ID, DSOS_CREATE | DSOS_EXCL, 1);
  ERROR_HANDLER(empty_sem, "Error opening producers_sem");
  consumers_sem = disastrOS_openSemaphore(CONSUMERS_SEM_ID, DSOS_CREATE | DSOS_EXCL, 1);
  ERROR_HANDLER(empty_sem, "Error opening consumers_sem");

  printf("[*] Created semaphores\n");
  disastrOS_printStatus();

  printf("[+] Creating %d producers and %d consumers\n", PRODUCERS_NUM, CONSUMERS_NUM);

  int alive_children=0;
  int job_type;
  for (int i = 0, job_type = PRODUCERS_SEM_ID; i<PRODUCERS_NUM; ++i) {
    disastrOS_spawn(childFunction, &job_type);
    alive_children++;
  }
  // for (int i = 0, job_type = CONSUMERS_SEM_ID; i<CONSUMERS_NUM; ++i) {
  //   disastrOS_spawn(childFunction, &job_type);
  //   alive_children++;
  // }

  disastrOS_printStatus();
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, alive_children);
    --alive_children;
  }

  printf("[-] Removing semaphores\n");
  disastrOS_closeSemaphore(empty_sem);
  disastrOS_closeSemaphore(fill_sem);
  disastrOS_closeSemaphore(producers_sem);
  disastrOS_closeSemaphore(consumers_sem);
  disastrOS_printStatus();

  printf("Shutdown!\n");
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
  printf("the function pointer is: %p\n", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
