#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include <random>

using namespace std;

int main() {
    int fd1[2];
    if (pipe(fd1) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Semaphore initialization
    sem_unlink("ss1");
    sem_t* sem= sem_open("ss1", O_CREAT | O_EXCL, 0644, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        // Handle semaphore creation failure
        // You might want to add error handling or exit the program
    }
    // Create child process
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        close(fd1[0]); // Close unused read end of the pipe
        // Generate a random integer
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dist(1, 100);
        int random_int = 69;
        sem_wait(sem); 
        write(fd1[1], &random_int, sizeof(random_int));
        int sval;
        sem_getvalue(sem, &sval);
        printf("Semavalue: %d\n", sval);
        
        
        // close(fd1[1]); // Close write end of the pipe in child

        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        close(fd1[1]); // Close unused write end of the pipe
        int sval;
        sem_getvalue(sem, &sval);
        printf("Semavalue: %d\n", sval);



        // ***************
        // ***************
        // ***************
        // ***************
        // ***************
        // int temp_val;
        // scanf("%d", &temp_val);
        sem_post(sem);



        int received_int;
        read(fd1[0], &received_int, sizeof(received_int));
        printf("Received integer from child process: %d\n" ,received_int);
        // close(fd1[0]); 
        wait(NULL);
        // sem_destroy(sem); 
        exit(EXIT_SUCCESS);
    }

    return 0;
}
