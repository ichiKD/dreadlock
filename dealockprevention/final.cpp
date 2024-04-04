#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <string.h>
#include <sstream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <semaphore.h>




std::vector<sem_t> semaphores;







int main(){
    int resources, process;
    int maximum[100][100];
    int avaliable[100];
    int deadline[100];
    int computation_time[100];

    // Input
    scanf("%d%d", &resources , &process);
    for(int i=0; i<resources; i++){
        scanf("%d", &avaliable[i]);
    }
    for(int i=0; i<process; i++){
        for(int j=0; j<resources; j++){
            scanf("%d", &maximum[i][j]);
        }
    }
    for(int i=0; i<process; i++){
        scanf("%d", &deadline[i]);
        computation_time[i] =0;
    }
    bool dreadlock_check=0;
    for(int i=0; i<process; i++){
        for(int j=0; j<resources; j++){
            if(maximum[i][j] > avaliable[j]){
                dreadlock_check =1; // will have dreadlock
            }
        }
    }


    for(int i=0; i<resources; i++){
        sem_t newSemaphore;
        sem_init(&newSemaphore, 0, 1); // Initialize the new semaphore
        semaphores.push_back(newSemaphore);
    }


    // computation_time = number of request and release 
    //                    + 




    return 0;
}