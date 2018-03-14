#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semClose(){
  //1 retrieve the sem of the resource to close
  int sem_fd = running->syscall_args[0];

  SemDescriptor* semDesc =  SemDescriptorList_byFd(&running->sem_descriptors, sem_fd);
  
  //2 if the sem is not in the the process, return an error
  if(!semDesc){
    printf("[!] Semaphore not in process\n");
    running->syscall_retvalue=DSOS_ESEMAPHORECLOSE;
    return;
  }

  //3 we remove the descriptor from the process list
  semDesc = (SemDescriptor*) List_detach(&running->sem_descriptors,  &(semDesc->list));
  assert(semDesc);

  Semaphore* sem=semDesc->semaphore;

  // we remove the descriptor pointer from the resource list
  SemDescriptorPtr* semDesPtr=(SemDescriptorPtr*) List_detach(&(sem->descriptors),(ListItem*)(semDesc->ptr));
  assert(semDesPtr);
  
  printf("Remaning descriptors %d for sem %d\n", sem->descriptors.size, sem->id);

  if(sem->descriptors.size == 0){
    printf("[-] Removing sem %d\n",sem->id);
    List_detach(&semaphores_list, (ListItem*)sem);
  }

  SemDescriptor_free(semDesc);
  SemDescriptorPtr_free(semDesPtr);
  running->syscall_retvalue=0;
}