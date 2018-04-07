# disastrOS-semaphores
An implementation for the **disastrOS** operating system of all the syscalls needed to manage semaphores

## Syscalls
`disastrOS_openSemaphore(int sem_id, int oflag, int count)`  
This is the syscall in charge of the creation and the opening of a semaphore  
*Assigned to:* [@andrea_tulimiero](https://github.com/andreatulimiero)

`int disastrOS_semPost(int sem_id)`

This is the syscall in charge of signal(semaphore) operation.  
*Assigned to:* [@daniele paliotta](https://github.com/dpstart)

`int disastrOS_semWait(int sem_id)`
This is the syscall in charg of wait(semaphore) operation.
*Assigned to:* [@luca ferrera](https://github.com/Luca-Ferrera)

'int disastrOS_closeSemaphore(int sem_id)'
This is the syscall in charg of close(semaphore) operation.
*Assigned to:* [@RiccardoBianchini](https://github.com/RiccardoBianchini)