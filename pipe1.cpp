#include <iostream>
#include <sys/types.h>
#include <sys/wait.h> 
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
#include <queue>




int resources, process;
int maximum[100][100];
int avaliable[100];
int deadline[100];
int computation_time[100];
bool done[100];
int processEnded;




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


struct process{
    int id;
    std::vector<std::vector<int>> index_to_accquired_resources;
};


std::vector<struct Instructions> processInstructions; 
std::vector<int> index_of_last_request_yet_to_be_processed;

std::vector< std::vector<std::string> > resourceList;
std::vector< std::vector< std::pair< sem_t, std::string> > > resourceListSemaphore;

std::vector<sem_t> processSemaphore;


void read_text(){
    std::ifstream file("example.txt"); // Open the file

    if (!file.is_open()) {
        printf("Error opening file\n");
        return;
    }

    std::string line = "";
    while(line == ""){
        std::getline(file, line);
    }
    resources = std::stoi(line);
    line = "";
    while(line == ""){
        std::getline(file, line);
    }
    process = std::stoi(line);


    for(int i=0; i<resources; i++){
        line = "";
        while(line == ""){
            std::getline(file, line);
        }
        int idx=0;
        if(line[0]<'0' || line[0]> '9'){
            for(int j=0; j<line.length(); j++){
                if(line[j] == '='){
                    idx = j+1;
                    break;
                }
            }
        }
        avaliable[i] = std::stoi(line.substr(idx));
    }

    for(int i=0; i<process; i++){
        line = "";
        while(line == ""){
            std::getline(file, line);
        }
        if(line[0]<'0' || line[0]> '9'){
            maximum[i][0] = std::stoi(line.substr(line.find('=') + 1));
            for(int j=1; j<resources; j++){
                line = "";
                while(line == ""){
                    std::getline(file, line);
                }
                maximum[i][j] = std::stoi(line.substr(line.find('=') + 1));
            }
        }
        else{
            std::istringstream int_stream(line);
            int x;
            for(int j=0; j<resources; j++){
                int_stream>> x;
                maximum[i][j] = x;
            }
        }
    }



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
            line = "";
            while(line == ""){
                std::getline(file, line);
            }
            deadline[end_count] = std::stoi(line);
            line = "";
            while(line == ""){
                std::getline(file, line);
            }
            computation_time[end_count] = std::stoi(line);
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


void read_resources(){
    std::ifstream file("example2.txt"); // Open the file

    if (!file.is_open()) {
        printf("Error opening file.");
        return;
    }

    std::string line="";
    for(int i=0; i<resources; i++){
        std::vector<std::string> r;
        line="";
        while(line == ""){
            std::getline(file, line);
        }
        line = line.substr(line.find(':')+1);
        printf("The line after 1st is %s\n", line.c_str());
        line = line.substr(line.find(':')+1);
        printf("The line after 2nd is %s\n", line.c_str());
        std::istringstream res(line);
        std::string item;
        while (std::getline(res, item, ',')) {
            item.erase(0, item.find_first_not_of(" ")); //Trimping starting space ' '
            item.erase(item.find_last_not_of(" ") + 1); //Trimping ending space   ' '
            r.push_back(item);
        }
        resourceList.push_back(r);
        r.clear();
    }
    file.close(); // Close the file

}


void print_read_text(){
    printf("the resources is %d, the procses is %d\n", resources, process);
    printf("The avaliable resourses are: \n");
    for(int i=0; i<resources; i++){
        printf("%d ", avaliable[i]);
    }
    printf("\n");
    printf("The maximum resourses needed are: \n");
    printf("    ");
    for(int i=0; i<resources; i++){
        printf("%2d ", i+1);
    }
    printf("\n");
    for(int i=0; i<process; i++){
        printf("%2d: ", i);
        for(int j=0; j<resources; j++){
            printf("%2d ", maximum[i][j]);
        }
        printf("\n");
    }
    printf("The avaliable deadline are: \n");
    for(int i=0; i<process; i++){
        printf("%2d ", deadline[i]);
    }
    printf("\n");
    printf("The avaliable computation_time are: \n");
    for(int i=0; i<process; i++){
        printf("%2d ", computation_time[i]);
    }
    printf("\n");
    for(int i=0; i<process; i++){
        printf("For %2dth process the instruction list is:\n", i+1);
        for(auto x: processInstructions[i].Ins){
            printf("{%d, {", x.first);
            for(int y : x.second){
                printf("%2d, ", y);
            }
            printf("}}\n");
        }

    }

    printf("The resourceList is\n");
    for(auto r: resourceList){
        for(std::string item: r){
            printf("%s\n", item.c_str());
        }
        printf("\n");
    }
}


void resources_to_semaphores(){
    for(auto r: resourceList){
        std::vector<std::pair<sem_t, std::string>> t;
        for(std::string item: r){
            std:: pair<sem_t, std::string> ss; 
            sem_init(&(ss.first), process, 1);
            t.push_back(ss);
        }
        resourceListSemaphore.push_back(t);
    }
}

void process_semaphores(){
    for(int i=0; i<process; i++){
        sem_t semaphore;
        sem_init(&semaphore, 1, 0);
        processSemaphore.push_back(semaphore);
    }
}



int main(){


    read_text();
    read_resources();
    resources_to_semaphores();
    print_read_text();






    for(int i=0; i<process; i++){
        index_of_last_request_yet_to_be_processed.push_back(0);
    }



    // {Earliest Deadline, {-longestJobFirst, processID}}
    std::priority_queue<std::pair<int, std::pair<int, int>>, 
        std::vector<std::pair<int, std::pair<int, int>>>, 
        std::greater<std::pair<int, std::pair<int, int>>> 
    > pq;


    // computation_time = number of request and release 
    //                    + parenthesized value in calculate + x from use_resources 



    // Earliest Deadline + longestJobFirst
    
    // id = process for schedular
    // for ith process ID = i-1
    int ID = process;
    pid_t pid;
    std::vector<pid_t> processID;
    for(int i=0; i<process; i++){
        pid = fork();
        if (pid == -1) {
            // Error occurred
            perror("fork");
            return 1;
        } 
        else if (pid == 0) {
            // This is the child process
            ID = i ;
            printf("Child process: PID=%d, Parent PID=%d\n", getpid(), getppid());
            break; // Child process terminates
        } 
        else {
            // This is the parent process
            printf("Parent process: Created child with PID=%d\n", pid);
            processID.push_back(pid);
        }
    }

    if(pid != 0){
        process_semaphores();
        for(int id=0; id<process; id++){
            pq.push({deadline[id], {-computation_time[id], id}});
        }
        processEnded = 0;
        int last_requested_process = -1;
        while (processEnded < process){
            
            
            
            
            
            
            
            
            
        }
    }
    else{
        int current_instruction=0;
        while(current_instruction < processInstructions[ID].Ins.size()){
            //request
            if(processInstructions[ID].Ins[current_instruction].first == 1){
                //Send Request
                sem_wait(&processSemaphore[ID]);
                //If request Successful, move along
                //Else current_instruction, continue
            }
            else{
                
            }
        }
    }
    return 0;
}