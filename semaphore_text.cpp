#include <iostream>
#include <vector>
#include <fcntl.h>        
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

// sem_open()
// sem_wait()
// sem_post()
// sem_unlink()
// sem_close()

// //Remove old semaphores
// sem_unlink("OLD_semaphores_NAME")
// sem_t *semphore = sem_open("NEW_NAME", IPC_CREAT, ) 


int main(){
    sem_unlink("OLD_semaphores_NAME");
    int value=0;
    sem_t *semaphore = sem_open("NEW_NAME", O_CREAT, 0660, value); 
    if(semaphore == SEM_FAILED){
        perror("NEW_NAME");
        exit(EXIT_FAILURE);
    }
    pid_t pid = fork();
    if(pid == 0){
        sem_wait(semaphore);
        printf("GO GO GO \n");
        fflush(stdout);
    }
    else{
        printf("plz GO\n");
        fflush(stdout);
    }
    while(wait(NULL) != -1 || errno != ECHILD){
        ;
    }
    sem_close(semaphore);
    sem_unlink("NEW_NAME");
    return 0;
}