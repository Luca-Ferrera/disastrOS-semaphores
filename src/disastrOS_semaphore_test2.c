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

#define READERS_NUM 5
#define WRITERS_NUM 5
#define WRITERS_SEM_ID 0
#define MUTEX_SEM_ID 2
#define BUFFER_SIZE 128

#define READER_CHILD 0
#define WRITER_CHILD 1

typedef struct Child_Args_s {
  int sem_id;
  int number;
} Child_Args_t;

int transactions[BUFFER_SIZE];  // circular buffer
int read_index;     // index of the next slot containing information to be read
int write_index;    // index of the next available slot for writing
int write_sem;
int mutex_sem;
int deposit;

int readcount = 0;

/** Writer **/
// TODO: Why does the func return a void* ?
void writerJob(int writer_no) {
  printf("[*]@Writer #%d\n", writer_no);
  int ret;
  int write_sem = disastrOS_openSemaphore(WRITERS_SEM_ID, 0);
  ERROR_HANDLER(write_sem, "Error opening write_sem in writerJob");
  int i = 0;
  while (i < 100) {
      // produce the item
      int currentTransaction = writer_no;

      ret = disastrOS_waitSemaphore(write_sem);
      ERROR_HANDLER(ret, "Error waiting write_sem in producerJob");

      transactions[write_index] = currentTransaction;
      write_index = (write_index + 1) % BUFFER_SIZE;

      ret = disastrOS_semPost(write_sem);
      ERROR_HANDLER(ret, "Error posting write_sem in producerJob");
    i++;
      
  }
  
  disastrOS_closeSemaphore(write_sem);
}

/** Reader **/
void readerJob(int reader_no) {
  printf("[*]@Reader #%d\n", reader_no);
  int ret;
  int write_sem = disastrOS_openSemaphore(WRITERS_SEM_ID, 0);
  ERROR_HANDLER(write_sem, "Error opening write_sem in readerJob");
  int mutex_sem = disastrOS_openSemaphore(MUTEX_SEM_ID, 0);
  ERROR_HANDLER(mutex_sem, "Error opening mutex_sem in readerJob");

  int i = 0;
  while (i < 100) {
  
    ret = disastrOS_waitSemaphore(mutex_sem);
    ERROR_HANDLER(ret, "Error waiting mutex_sem in producerJob");

    readcount++;

    if(readcount == 1){
        //if you are the first reader, lock the resource from writers.
        ret = disastrOS_waitSemaphore(write_sem);
        ERROR_HANDLER(ret, "Error waiting write_sem in producerJob");
    }
    ret = disastrOS_semPost(mutex_sem);
    ERROR_HANDLER(ret, "Error posting mutex_sem in producerJob");
    
        // read the item and update read_index accordingly
        int readTransaction = transactions[read_index];
        printf("Reader %d read something: %d\n", reader_no, readTransaction);
        read_index = (read_index + 1) % BUFFER_SIZE;

    readcount--;

    if(readcount == 0){
        //last reader, can unlock write semaphore
        ret = disastrOS_semPost(write_sem);
        ERROR_HANDLER(ret, "Error posting write_sem in producerJob");
    }

    i++;
  }
  
  disastrOS_closeSemaphore(write_sem);
  disastrOS_closeSemaphore(mutex_sem);
}

void childFunction(void* args){
  disastrOS_printStatus();
  Child_Args_t *child_args_t = (Child_Args_t*) args;
  if (child_args_t->sem_id  == READER_CHILD) {
    readerJob(child_args_t->number);
    printf("[-]@Child Reader #%d finished working\n", child_args_t->number);
  }
  else if (child_args_t->sem_id == WRITER_CHILD) {
    writerJob(child_args_t->number);
    printf("[-]@Child Writer #%d finished working\n", child_args_t->number);
  }

  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
  disastrOS_printStatus();

  printf("[+]@Init Creating semaphores ... \n");
  // Creating semaphores
  mutex_sem = disastrOS_openSemaphore(MUTEX_SEM_ID, DSOS_CREATE | DSOS_EXCL, BUFFER_SIZE);
  ERROR_HANDLER(mutex_sem, "Error opening mutex_sem");

  write_sem = disastrOS_openSemaphore(WRITERS_SEM_ID, DSOS_CREATE | DSOS_EXCL, 0);
  ERROR_HANDLER(write_sem, "Error opening write_sem");

  disastrOS_printStatus();

  printf("[*]@Init Starting transactions with %d readers and %d writers\n", READERS_NUM, WRITERS_NUM);

  int alive_children=0;

  Child_Args_t writers_args[WRITERS_NUM];
  Child_Args_t readers_args[READERS_NUM];

  for (int i = 0; i<WRITERS_NUM; ++i) {
    writers_args[i].sem_id = WRITER_CHILD;
    writers_args[i].number = i;
    disastrOS_spawn(childFunction, &writers_args[i]);
    alive_children++;
  }
  for (int i = 0; i<READERS_NUM; ++i) {
    readers_args[i].sem_id = READER_CHILD;
    readers_args[i].number = i;
    disastrOS_spawn(childFunction, &readers_args[i]);
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
  disastrOS_closeSemaphore(mutex_sem);
  disastrOS_closeSemaphore(write_sem);
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
