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
    std::vector<std::pair<int, std::vector<int>>> Ins;
    // INS[i] = 
    // request(1, 2, 3, 4, .. m) = {1, {1, 2, 3, ... m}}
    // release(1, 2, 3, 4, .. m) = {2, {1, 2, 3, ... m}}
    // calculate(x) = {3, {x}}
    // use_resorusces(x, y) = {4, {x, y}}
    // print_recouses_used = {5, {}}
    // end = {6, {}}
};





std::vector<struct Instructions> processInstructions; 



void read_text(){
    std::ifstream file("example.txt"); // Open the file

    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return;
    }

    std::string line;
    int end_count=0;
    struct Instructions Temp;
    while (end_count < process) { // Read lines until the end of the file
        std::getline(file, line);
        while(line == ""){
            std::getline(file, line);
        }
        if(line == "end."){
            end_count++;
            std::pair<int, std::vector<int>> order;
            order.first = 6;
            Temp.Ins.push_back(order);
            processInstructions.push_back(Temp);
            Temp.Ins.clear();
        }
        else if(line.substr(0, 7) == "request"){
            std::pair<int, std::vector<int>> order;
            order.first = 1;
            for(int i=8; i<line.length(); i++){
                while( (line[i] == ' ' || line[i] == ',' || line[i] == '(' ||
                        line[i] == ')' || line[i] == ';'  ) 
                        && i<line.length() ){
                    i++;
                }
                int x=0;
                while('0'<=line[i] && line[i] <= '9'){
                    x=x*10 + (line[i] - '0');
                    i++;
                }
                order.second.push_back(x);
                if(order.second.size() == resources){
                    break;
                }
            }
            if(order.second.size() != resources){
                printf("Error in reading request\n");
            }
            Temp.Ins.push_back(order);
        }
        else if(line.substr(0, 7) == "release"){
            std::pair<int, std::vector<int>> order;
            order.first = 2;
            for(int i=8; i<line.length(); i++){
                while( (line[i] == ' ' || line[i] == ',' || line[i] == '(' ||
                        line[i] == ')' || line[i] == ';'  ) 
                        && i<line.length() ){
                    i++;
                }
                int x=0;
                while('0'<=line[i] && line[i] <= '9'){
                    x=x*10 + (line[i] - '0');
                    i++;
                }
                order.second.push_back(x);
                if(order.second.size() == resources){
                    break;
                }
            }
            if(order.second.size() != resources){
                printf("Error in reading release\n");
            }
            Temp.Ins.push_back(order);
        }
        else if(line.substr(0, 7) == "process"){
            continue;
        }
        else if(line.substr(0, 9) == "calculate"){
            std::pair<int, std::vector<int>> order;
            order.first = 3;
            for(int i=10; i<line.length(); i++){
                while( (line[i] == ' ' || line[i] == ',' || line[i] == '(' ||
                        line[i] == ')' || line[i] == ';'  ) 
                        && i<line.length() ){
                    i++;
                }
                int x=0;
                while('0'<=line[i] && line[i] <= '9'){
                    x=x*10 + (line[i] - '0');
                    i++;
                }
                order.second.push_back(x);
                if(order.second.size() == 1){
                    break;
                }
            }
            if(order.second.size() != 1){
                printf("Error in reading calculate\n");
            }
            Temp.Ins.push_back(order);
        }
        else if(line.substr(0, 13) == "use_resources"){
            std::pair<int, std::vector<int>> order;
            order.first = 4;
            for(int i=14; i<line.length(); i++){
                while( (line[i] == ' ' || line[i] == ',' || line[i] == '(' ||
                        line[i] == ')' || line[i] == ';'  ) 
                        && i<line.length() ){
                    i++;
                }
                int x=0;
                while('0'<=line[i] && line[i] <= '9'){
                    x=x*10 + (line[i] - '0');
                    i++;
                }
                order.second.push_back(x);
                if(order.second.size() == 2){
                    break;
                }
            }
            if(order.second.size() != 2){
                printf("Error in reading use_resources\n");
            }
            Temp.Ins.push_back(order);
        }
        else if(line == "print_resources_used;"){
            std::pair<int, std::vector<int>> order;
            order.first = 5;
            Temp.Ins.push_back(order);
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