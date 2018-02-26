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
  printf("PID %d requested open of semaphore %d\n", running->pid, sem_id);

  Semaphore* sem = SemaphoreList_byId(&running->sem_descriptors, sem_id);
  if (sem) {
    printf("Semaphore with id %d already existing, returning it\n", sem_id);
    running->syscall_retvalue = sem;
  }

  printf("Semaphore with id %d doesn't exist, allocating it\n", sem_id);
  running->syscall_retvalue = Semaphore_alloc(sem_id, count);
}
