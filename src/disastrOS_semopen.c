#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semOpen(){
  int sem_id = running->syscall_args[0];
  int count = running->syscall_args[1];
  printf("[*] PID %d requested open of semaphore %d\n", running->pid, sem_id);

  // Check if a semaphore with the requested ID is already open
  sem = SemaphoreList_byId(&semaphores_list, sem_id);
  if (!sem) {
    printf("[+] Semaphore with id %d doesn't exist, allocating it\n", sem_id);
    sem = Semaphore_alloc(sem_id, count);
    // Add created semaphore to global list
    List_insert(&semaphores_list, semaphores_list.last, &(sem->list));
  }

  SemDescriptor* desc = SemDescriptor_alloc(running->last_sem_fd, sem, running);

  ++(running->last_sem_fd);
  SemDescriptorPtr* desc_ptr = SemDescriptorPtr_alloc(desc);
  List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*) desc_ptr);

  desc->ptr = desc_ptr;
  List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) desc_ptr);

  running->syscall_retvalue = sem->id;
}
