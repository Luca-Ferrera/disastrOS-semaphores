#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"
#include "disastrOS_semaphore.h"

#define ERROR_HANDLER(ret, msg) \
  do {\
    if (ret < 0) { \
      fprintf(stderr, "[!](%d) %s\n", ret, msg); \
      disastrOS_exit(1); \
    }\
  } while(0);

#define PRODUCERS_NUM 1
#define CONSUMERS_NUM 1
#define EMPTY_SEM_ID 0
#define FILL_SEM_ID 1
#define PRODUCERS_SEM_ID 2
#define CONSUMERS_SEM_ID 3
#define BUFFER_SIZE 8
#define TRANSACTIONS_NUMBER 5

typedef struct Child_Args_s {
  int sem_id;
  int number;
} Child_Args_t;

int transactions[BUFFER_SIZE];  // circular buffer
int read_index;     // index of the next slot containing information to be read
int write_index;    // index of the next available slot for writing
int empty_sem;
int fill_sem;
int consumers_sem;
int producers_sem;
int deposit;

/** Producer **/
void producerJob(int producer_no) {
  printf("[*]@Producer #%d\n", producer_no);
  int ret;
  int empty_sem = disastrOS_openSemaphore(EMPTY_SEM_ID, 0);
  ERROR_HANDLER(empty_sem, "Error opening empty_sem in producerJob");
  int producers_sem = disastrOS_openSemaphore(PRODUCERS_SEM_ID, 0);
  ERROR_HANDLER(producers_sem, "Error opening producers_sem in producerJob");
  int i = 0;
  while (i < TRANSACTIONS_NUMBER) {
      // produce the item
      int currentTransaction = 1;

      ret = disastrOS_semWait(empty_sem);
      ERROR_HANDLER(ret, "Error waiting empty_sem in producerJob");

      ret = disastrOS_semWait(producers_sem);
      ERROR_HANDLER(ret, "Error waiting producers_sem in producerJob");

      transactions[write_index] = currentTransaction;
      write_index = (write_index + 1) % BUFFER_SIZE;

      ret = disastrOS_semPost(producers_sem);
      ERROR_HANDLER(ret, "Error posting producers_sem in producerJob");

    i++;

      ret = disastrOS_semPost(fill_sem);
      ERROR_HANDLER(ret, "Error posting fill_sem in producerJob");

    i++;
  }

  ret = disastrOS_closeSemaphore(empty_sem);
  ERROR_HANDLER(ret, "Error closing empty_sem in producerJob");
  ret = disastrOS_closeSemaphore(producers_sem);
  ERROR_HANDLER(ret, "Error closing producers_sem in producerJob");
}

/** Consumer **/
void consumerJob(int consumer_no) {
  printf("[*]@Consumer #%d\n", consumer_no);
  int ret;
  int fill_sem = disastrOS_openSemaphore(FILL_SEM_ID, 0);
  ERROR_HANDLER(fill_sem, "Error opening fill_sem in consumerJob");
  int consumers_sem = disastrOS_openSemaphore(CONSUMERS_SEM_ID, 0);
  ERROR_HANDLER(consumers_sem, "Error opening consumers_sem in consumerJob");
  int i = 0;
  while (i < TRANSACTIONS_NUMBER) {
      ret = disastrOS_semWait(fill_sem);
      ERROR_HANDLER(ret, "Error waiting fill_sem in consumerJob");

      ret = disastrOS_semWait(consumers_sem);
      ERROR_HANDLER(ret, "Error waiting consumers_sem in consumerJob");

      // get the item and update read_index accordingly
      int lastTransaction = transactions[read_index];
      deposit += lastTransaction;
      read_index = (read_index + 1) % BUFFER_SIZE;

      ret = disastrOS_semPost(consumers_sem);
      ERROR_HANDLER(ret, "Error posting consumers_sem in consumerJob");

      ret = disastrOS_semPost(empty_sem);
      ERROR_HANDLER(ret, "Error posting empty_sem in consumerJob");

      if (read_index % 10 == 0) {
          printf("After the last 10 transactions balance is now %d.\n", deposit);
      }
      i++;
    // disastrOS_sleep(1);
  }

  ret = disastrOS_closeSemaphore(fill_sem);
  ERROR_HANDLER(ret, "Error closing fill_sem in consumerJob");
  ret = disastrOS_closeSemaphore(consumers_sem);
  ERROR_HANDLER(ret, "Error closing consumers_sem in consumerJob");
}

void childFunction(void* args){
  disastrOS_printStatus();
  Child_Args_t *child_args_t = (Child_Args_t*) args;
  if (child_args_t->sem_id  == PRODUCERS_SEM_ID) {
    producerJob(child_args_t->number);
    printf("[-]@Child Producer #%d finished working\n", child_args_t->number);
  }
  else if (child_args_t->sem_id == CONSUMERS_SEM_ID) {
    consumerJob(child_args_t->number);
    printf("[-]@Child Consumer #%d finished working\n", child_args_t->number);
  }

  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
  int ret;
  disastrOS_printStatus();

  printf("[+]@Init Creating semaphores ... \n");
  // Creating empty and fill semaphores
  empty_sem = disastrOS_openSemaphore(EMPTY_SEM_ID, DSOS_CREATE | DSOS_EXCL, BUFFER_SIZE);
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
  disastrOS_printStatus();

  printf("[*]@Init Starting transactions with %d consumers and %d producers\n", CONSUMERS_NUM, PRODUCERS_NUM);

  int alive_children=0;

  Child_Args_t producers_args[PRODUCERS_NUM];
  Child_Args_t consumers_args[CONSUMERS_NUM];

  for (int i = 0; i<PRODUCERS_NUM; ++i) {
    producers_args[i].sem_id = PRODUCERS_SEM_ID;
    producers_args[i].number = i;
    disastrOS_spawn(childFunction, &producers_args[i]);
    alive_children++;
  }
  for (int i = 0; i<CONSUMERS_NUM; ++i) {
    consumers_args[i].sem_id = CONSUMERS_SEM_ID;
    consumers_args[i].number = i;
    disastrOS_spawn(childFunction, &consumers_args[i]);
     alive_children++;
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){
    // disastrOS_printStatus();
    printf("[-]@Init child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, alive_children);
    --alive_children;
  }

  printf("[-] Removing semaphores\n");
  ret = disastrOS_closeSemaphore(empty_sem);
  ERROR_HANDLER(ret, "Error closing empty_sem");
  ret = disastrOS_closeSemaphore(fill_sem);
  ERROR_HANDLER(ret, "Error closing fill_sem");
  ret = disastrOS_closeSemaphore(producers_sem);
  ERROR_HANDLER(ret, "Error closing producers_sem");
  ret = disastrOS_closeSemaphore(consumers_sem);
  ERROR_HANDLER(ret, "Error closing consumers_sem");

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
