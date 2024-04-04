#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <cerrno>
#include <vector>

std::vector<pid_t> processID;


struct Instructions
{
    std::vector<std::pair<int, std::vector<int>>> INS;
    // INS[i] = 
    // calculate(x) = {1, {x}}
    // request(1, 2, 3, 4, .. m) = {2, {1, 2, 3, ... m}}
    // use_resorusces(x, y) = {3, {x, y}}
    // release(1, 2, 3, 4, .. m) = {4, {1, 2, 3, ... m}}
    // print_recouses_used = {5, {}}
    // end = {6, 0}
};





int main() {
    pid_t pid = fork();
    



    // Parent process waits for all children to terminate
    while(wait(NULL) != -1 || errno != ECHILD){
        ;
    }

    return 0;
}
