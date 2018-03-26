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
  printf("Sem_desc %x \n", sem_desc);
  if(!sem_desc){
    printf("errore\n");
    running->syscall_retvalue = DSOS_ESEMAPHOREWAIT;
    return;
  }
  printf("no errore\n");
  
  Semaphore* sem = sem_desc->semaphore;

  sem->count-=1;
  if(sem->count <= 0){
    //put running process into waiting_list
    printf("Semaphore id: %x\n",sem->id);
    printf("Semaphore waiting list: %x\n",sem->waiting_descriptors);
    printf("Semaphore waiting list last: %x\n",sem->waiting_descriptors.last);
    printf("running process: %x\n",running->pid);
    printf("running next: %x\n",((ListItem*)running)->next);
    printf("running prev: %x\n",((ListItem*)running)->prev); 
    
      
    ListItem* l = List_find(&running->sem_descriptors, (ListItem*) sem_desc);
    
    printf("running sem_desc_last: %x\n",l);   
    List_insert(&(sem->waiting_descriptors), sem->waiting_descriptors.last, l);
    //running->status = Waiting;
    List_insert(&waiting_list, waiting_list.last, (ListItem*)running);

    //running = (PCB*) List_detach(&ready_list,(ListItem*)ready_list.last);
    //scheduling next process
    //disastrOS_printStatus();
    //disastrOS_preempt();
    
  }
  running->syscall_retvalue=0;
}
