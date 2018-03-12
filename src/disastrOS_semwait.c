#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semWait(){
  int sem_id = running->syscall_args[0];
  Semaphore* sem = SemaphoreList_byId(&semaphores_list, sem_id);
  if(!sem){
    running->syscall_retvalue = DSOS_ERESEMAPHORE;
    return;
  }
  sem->count-=1;
  if(sem->count < 0){
    //put running process into waiting_list
    List_insert(&waiting_list, waiting_list.last, (ListItem*)running);
    //scheduling next process
    disastrOS_preempt();
  }
  running->syscall_retvalue=0;
}
