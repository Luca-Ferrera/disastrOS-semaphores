#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

unsigned char semaphore_pool_allocator_init = 0; // global variable for Semaphore structures intialization

void internal_semOpen(){
  Semaphore* sem = (Semaphore*) running->syscall_args[0];
  int sem_id = running->syscall_args[1];
  int count = running->syscall_args[2];
  printf("[*] PID %d requested open of semaphore %d\n", running->pid, sem_id);

  // Initialize the semaphore once
  if (!(semaphore_pool_allocator_init++)) Semaphore_init();

  // Check if a semaphore with the same ID is already open
  sem = SemaphoreList_byId(&semaphores_list, sem_id);
  if (sem) {
    printf("[*] Semaphore with id %d already exists, returning it\n", sem_id);
    running->syscall_retvalue = sem->id;
    return;
  }

  printf("[+] Semaphore with id %d doesn't exist, allocating it\n", sem_id);
  sem = Semaphore_alloc(sem_id, count);
  // Add created semaphore to global list
  List_insert(&semaphores_list, semaphores_list.last, &(sem->list));
  running->syscall_retvalue = sem->id;
}
