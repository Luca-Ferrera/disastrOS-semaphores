#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

unsigned char initialized = 0;

void internal_semOpen(){
  Semaphore* sem = (Semaphore*) running->syscall_args[0];
  int sem_id = running->syscall_args[1];
  int count = running->syscall_args[2];
  printf("PID %d requested open of semaphore %d\n", running->pid, sem_id);

  // Initialize the semaphore once
  if (!(initialized++)) Semaphore_init();

  // Check if a semaphore with the same ID is already open
  sem = SemaphoreList_byId(&running->sem_descriptors, sem_id);
  if (sem) {
    printf("Semaphore with id %d already existing, returning it\n", sem_id);
    running->syscall_retvalue = sem->id;
  }

  // Create a semaphore with the given ID
  printf("Semaphore with id %d doesn't exist, allocating it\n", sem_id);
  sem = Semaphore_alloc(sem_id, count);
  running->syscall_retvalue = sem->id;
}
