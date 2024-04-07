#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <chrono>
#include <vector>
#include <queue>
#include <algorithm>
#include <fcntl.h> 
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>

int resources, process;
int maximum[100][100];
int avaliable[100];
int deadline[100];
int computation_time[100];
bool done[100];
int endcount;




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

std::vector<std::vector<std::string> > resourceList;
std::vector<std::vector<sem_t *>> resourceListSemaphore;
std::vector<std::vector<int>> resourceListCheck;

std::vector<sem_t*> processSemaphore;
sem_t* schedulerSemaphore;

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
        if(line.substr(0, 4) == "end."){
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
        else if(line.substr(0, 20)  == "print_resources_used"){
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
        std::vector<sem_t*> t;
        for(std::string item: r){
            sem_unlink(item.c_str());
            sem_t *ss = sem_open(item.c_str(), O_CREAT, 0660, 0);
            if(ss == SEM_FAILED){
                perror(item.c_str());
                exit(EXIT_FAILURE);
            }
            t.push_back(ss);
        }
        resourceListSemaphore.push_back(t);
    }
}

void process_semaphores() {
    for (int i = 0; i < process; ++i) {
        std::string name = "Process_ID_" + std::to_string(i);
        sem_unlink(name.c_str());
        // Create semaphore with unique name
        sem_t *ss = sem_open(name.c_str(), O_CREAT | O_EXCL, 0644, 0);
        if (ss == SEM_FAILED) {
            perror("sem_open");
            // Handle semaphore creation failure
            // You might want to add error handling or exit the program
        } else {
            // Semaphore created successfully, add to vector
            printf("SUCCESS");
            processSemaphore.push_back(ss);
        }
    }
}





int main(){



    read_text();
    read_resources();
    resources_to_semaphores();
    print_read_text();
    printf("THe number of proces are %d\n", process);
    process_semaphores();
    printf("The size of processSemaphore is  %ld", processSemaphore.size());
    fflush(stdout);
    const char* SHARED_MEMORY_NAME = "/my_shared_memory";
    int shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(int) * resources);
    int* shared_numbers = static_cast<int*>(mmap(NULL, sizeof(int) * resources, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));

    for (int i = 0; i < resources; ++i) {
        shared_numbers[i] = avaliable[i];   // Set elements to avaliable
    }

    // {Earliest Deadline, {-longestJobFirst, processID}}
    std::priority_queue<std::pair<int, std::pair<int, int>>, 
        std::vector<std::pair<int, std::pair<int, int>>>, 
        std::greater<std::pair<int, std::pair<int, int>>> 
    > pq;




/*
    pipes
    fd1 - sends_id
    fd2 - sends_computation_time
    fd3 - sends_vector_request

    fd1, fd2, fd3
    parent       child
    read   ----  write

    fd4 - sends_id
    fd5 - sends request approval/rejection
    fd6 - sends_index_of_allocated_resources
    child        parent
    read   ----  write

*/


    int fd1[2], fd2[2], fd3[2], fd4[2], fd5[2], fd6[2]; 


    if(pipe(fd1) == -1){
        printf("Error in making a pipe\n");
        fflush(stdout);
    }
    if(pipe(fd2) == -1){
        printf("Error in making a pipe\n");
        fflush(stdout);
    }
    if(pipe(fd3) == -1){
        printf("Error in making a pipe\n");
        fflush(stdout);
    }
    if(pipe(fd4) == -1){
        printf("Error in making a pipe\n");
        fflush(stdout);
    }
    if(pipe(fd5) == -1){
        printf("Error in making a pipe\n");
        fflush(stdout);
    }
    if(pipe(fd6) == -1){
        printf("Error in making a pipe\n");
        fflush(stdout);
    }

    // Semaphore initialization
    sem_unlink("ss1");
    sem_t* sem= sem_open("ss1", O_CREAT | O_EXCL, 0644, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
    }
    sem_unlink("ss2");
    sem_t* sem2= sem_open("ss2", O_CREAT | O_EXCL, 0644, 0);
    if (sem2 == SEM_FAILED) {
        perror("sem_open");
    }



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
            // printf("Child process: ID=%d, PID=%d, Parent PID=%d\n", ID, getpid(), getppid());
            // fflush(stdout);
            break; // Child process terminates
        } 
        else {
            // This is the parent process
            // printf("Parent process: Created child with PID=%d\n", pid);
            // fflush(stdout);
            processID.push_back(pid);
        }
    }



    printf("\n");
    printf("%d %d\n", ID, process);
    fflush(stdout);
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } 




    if(ID==process){
        printf("The number of process are %d\n", process);
        fflush(stdout);
        for(int id=0; id<process; id++){
            pq.push({deadline[id], {-computation_time[id], id}});
        }
        endcount = 0;
        int last_process = -1;
        int main_process_in_bankers_algo =-1;
        auto pq_temp=pq;
        while(!pq_temp.empty()){
            auto y = pq_temp.top();
            pq_temp.pop();
            printf("%d %d %d\n", y.first, y.second.first, y.second.second);
        }
        fflush(stdout);
        // while(!pq.empty()){
        //     auto y = pq.top();
        //     pq.pop();
        //     printf("%d %d %d\n", y.first, y.second.first, y.second.second);
        // }
        // fflush(stdout);
        while (endcount < process){
            if(main_process_in_bankers_algo == -1){
                auto current= pq.top();
                pq.pop();
                int idx =current.second.second;
                sem_post(processSemaphore[current.second.second]);
                std::vector<int> b(resources) ;
                read(fd1[0], b.data(), sizeof(int)*resources);
                printf("Received vector from child process%d:\n", idx);

                for(int x: b){
                    printf("%d ", x);
                }
                printf("\n");
                fflush(stdout);

            }
        }
    }
    else{
        printf("The child process id is %d\n", ID);
        int current_instruction=0;
        std::vector<std::vector<std::string>> master_string;
        std::vector<std::vector<sem_t *>> master_sem_t;
        for(int i=0; i<resources; i++){
            std::vector<std::string> t;
            master_string.push_back(t);
        }
        int relative_time=0;
        int computationTime1=0;
        int first_request =1;
        while(current_instruction < processInstructions[ID].Ins.size()){
            printf("Process%d LOOP\n", ID);
            fflush(stdout);
            if(processInstructions[ID].Ins[current_instruction].first == 1){
                sem_wait(processSemaphore[ID]); 
                std::vector<int> a = processInstructions[ID].Ins[current_instruction].second;
                printf("the requet vector is:\n");
                for(auto x: a){
                    printf("%d ", x);
                }
                printf("\n");
                fflush(stdout);
                write(fd1[1], a.data(), sizeof(int)*resources);
            }
            else{
                ;
            }
            current_instruction++;
        }
        printf("Process%d ENDED\n", ID);
        fflush(stdout);
    }


    


    while (wait(NULL) != -1){
        ;
    }
    munmap(shared_numbers, sizeof(int) * resources);
    shm_unlink(SHARED_MEMORY_NAME);


    return 0;
}
