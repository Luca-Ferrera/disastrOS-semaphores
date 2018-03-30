#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semPost(){

  // The argument is the file descriptor of the semaphore
  int fd = running->syscall_args[0];

  // Get Semaphore Descriptor from file descriptor  
  SemDescriptor* sem_desc = SemDescriptorList_byFd(&running->sem_descriptors, fd);

  /*// Finally, get semaphore from SemDescriptor
  Semaphore* sem = SemaphoreList_byId(&semaphores_list, sem_id);
  if(!sem){
    running->syscall_retvalue = DSOS_ESEMAPHOREPOST;
    return;
  }*/

  Semaphore* sem = sem_desc->semaphore;
  sem->count++;

  if(sem->count <= 0){

    //Get descriptor from those waiting on this semaphore
    //and wake it up
    SemDescriptorPtr* sem_desc_ptr = (SemDescriptorPtr*)List_detach(&sem->waiting_descriptors, sem->waiting_descriptors.first);

    if(!sem_desc_ptr){
            // No semaphores in waiting list
            running->syscall_retvalue = DSOS_ERESOURCEOPEN;
            return;
    }

    SemDescriptor* sem_desc = sem_desc_ptr->descriptor;
    PCB* ready_process = sem_desc->pcb;

    PCB* ret = (PCB*)List_detach(&waiting_list, (ListItem*)ready_process);
    if(!ret){
        // The process is not in waiting list
        running->syscall_retvalue = DSOS_ERESOURCEOPEN;
        return;
    }
    
    // Set state of ready process
    ready_process->status = Ready;
    
    //Insert process in ready list
    List_insert(&ready_list, ready_list.last, (ListItem*)ready_process);
  }
  running->syscall_retvalue=0;
}
