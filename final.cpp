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




int resources, process;



std::vector<sem_t> semaphores;


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


std::vector<struct Instructions> process_I; 



void read_text(){
    std::ifstream file("example.txt"); // Open the file

    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return;
    }

    std::string line;
    int end_count=0;
    while (end_count < process) { // Read lines until the end of the file
        std::getline(file, line);
        while(line == ""){
            std::getline(file, line);
        }
        if(line == "end."){
            end_count++;
        }
        else if(line == "print_resources_used;"){

        }
        else if(line.substr(0, 7) == "process"){
            continue;
        }
        else if(line.substr(0, 7) == "request"){
            
        }
        else if(line.substr(0, 13) == "use_resources"){

        }
        else{
            const char* cLine = line.c_str();
            printf("Error occured while reading the below line\n");
            printf("%s\n", cLine);
        }
    }

    file.close(); // Close the file

}





int main(){
    int maximum[100][100];
    int avaliable[100];
    int deadline[100];
    int computation_time[100];
    bool done[100];

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
        done[i] = false;
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