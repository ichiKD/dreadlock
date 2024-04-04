#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <cerrno>
#include <vector>

std::vector<pid_t> processID;


int main() {
    int n = 5; // Number of processes to create
    // For ith child processes ID = will be i-1 {0, 1, ... n-1}
    // For manager process ID = n
    int ID = n;
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            // Error occurred
            perror("fork");
            return 1;
        } 
        else if (pid == 0) {
            // This is the child process
            ID =i;
            printf("Child process: PID=%d, Parent PID=%d\n", getpid(), getppid());
            break; // Child process terminates
        } 
        else {
            // This is the parent process
            printf("Parent process: Created child with PID=%d\n", pid);
            processID.push_back(pid);
        }
    }

    // Parent process waits for all children to terminate
    while(wait(NULL) != -1 || errno != ECHILD){
        ;
    }

    return 0;
}
