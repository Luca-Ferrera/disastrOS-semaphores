#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semOpen(){
  // (1) get from PCB the ID and optional count of the semaphore to be opened
  int sem_id = running->syscall_args[0];
  int oflag  = running->syscall_args[1];
  int count  = running->syscall_args[2];
  printf("[*] PID %d requested open of semaphore %d\n", running->pid, sem_id);

  // (2) Check if a semaphore with the requested ID is already opened
  Semaphore* sem = SemaphoreList_byId(&semaphores_list, sem_id);
  if (oflag & DSOS_CREATE) {
    if (sem) {
      running->syscall_retvalue = DSOS_ESEMAPHORECREATE;
      return;
    }
    printf("[+] Semaphore with id %d doesn't exist, allocating it\n", sem_id);
    sem = Semaphore_alloc(sem_id, count);
    // Add created semaphore to global list
    List_insert(&semaphores_list, semaphores_list.last, &(sem->list));
  }

  //  (3) Check that everything is okay
  // We should have the semaphore now, otherwise an error occured
  if (!sem) {
    running->syscall_retvalue = DSOS_ESEMAPHOREOPEN;
    return;
  }
  // If an exclusive opening was requested but other processes already retain the semaphore return an error
  if ((oflag & DSOS_EXCL) && sem->descriptors.size) {
    running->syscall_retvalue = DSOS_ESEMAPHORENOEXCL;
    return;
  }

  // (5) Create the descriptor for the resource in this process, and add it to
  //  the process descriptor list. Assign to the resource a new fd
  SemDescriptor* desc = SemDescriptor_alloc(running->last_sem_fd, sem, running);
  if (!desc){
     running->syscall_retvalue=DSOS_ERESOURCENOFD;
     return;
  }
  running->last_sem_fd++; // increment last sem_fd number
  List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*) desc);

  // (6) Append the SemDescriptorPtr in the list
  SemDescriptorPtr* desc_ptr = SemDescriptorPtr_alloc(desc);
  desc->ptr = desc_ptr;
  List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) desc_ptr);

  // return the created SemdDescriptor to the process
  // printf("[*] End of semOpen with id %d for process %d and flags %d\n", sem_id, running->pid, oflag);
  // SemaphoreList_print(&semaphores_list);
  running->syscall_retvalue = desc->fd;
}
