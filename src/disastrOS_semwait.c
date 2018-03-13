#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semWait(){
  int sem_fd = running->syscall_args[0];
  
  printf("Pid %d requesting semwait on %d semaphore\n", running->pid, sem_fd);

  SemDescriptor* sem_desc = SemDescriptorList_byFd(&(running->sem_descriptors), sem_fd);
  printf("Sem_desc %d \n", sem_desc);
  if(!sem_desc){
    printf("errore\n");
    running->syscall_retvalue = DSOS_ESEMAPHOREWAIT;
    return;
  }
  printf("no errore\n");
  //TODO: check if process owns the semaphore
  Semaphore* sem = sem_desc->semaphore;
  sem->count-=1;
  if(sem->count < 0){
    //put running process into waiting_list
    List_insert(&waiting_list, waiting_list.last, (ListItem*)running);
    List_insert(&(sem->waiting_descriptors),sem->waiting_descriptors.last,(ListItem*)sem_desc);
    //scheduling next process
    running->syscall_retvalue=0;
    printf("\nSemaphores: \n");
    SemaphoreList_print(&semaphores_list);
    printf("\nWaiting: \n");
    PCBList_print(&waiting_list);
    disastrOS_preempt();
  }
  running->syscall_retvalue=0;
}
