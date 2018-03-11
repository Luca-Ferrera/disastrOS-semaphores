#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

unsigned char semaphore_pool_allocator_init = 0; // global variable for Semaphore structures intialization
unsigned char sem_desc_pool_allocator_init = 0;

void internal_semOpen(){
  Semaphore* sem = (Semaphore*) running->syscall_args[0];
  int sem_id = running->syscall_args[1];
  int count = running->syscall_args[2];
  printf("[*] PID %d requested open of semaphore %d\n", running->pid, sem_id);

  // Initialize the semaphore pool allocatr once
  if (!semaphore_pool_allocator_init) {
    Semaphore_init();
    semaphore_pool_allocator_init = 1;
  }

  // Check if a semaphore with the requested ID is already open
  sem = SemaphoreList_byId(&semaphores_list, sem_id);
  if (!sem) {
    printf("[+] Semaphore with id %d doesn't exist, allocating it\n", sem_id);
    sem = Semaphore_alloc(sem_id, count);
    // Add created semaphore to global list
    List_insert(&semaphores_list, semaphores_list.last, &(sem->list));
  }

  // Initialize the sem desc pool allocatr once
  if (!sem_desc_pool_allocator_init) {
    SemDescriptor_init();
    sem_desc_pool_allocator_init = 1;
  }

  SemDescriptor* desc = SemDescriptor_alloc(running->last_sem_fd, sem, running);

  ++(running->last_sem_fd);
  SemDescriptorPtr* desc_ptr = SemDescriptorPtr_alloc(desc);
  List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*) desc_ptr);

  desc->ptr = desc_ptr;
  List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) desc_ptr);

  running->syscall_retvalue = sem->id;
}
