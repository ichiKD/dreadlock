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




struct Instructions{
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
std::vector<sem_t*> processSemaphore;
std::vector<std::vector<std::string> > resourceList;
std::vector<std::vector<sem_t *>> resourceListSemaphore;


using MyPriorityQueue = std::priority_queue<
    std::pair<int, std::pair<int, int>>,
    std::vector<std::pair<int, std::pair<int, int>>>,
    std::greater<std::pair<int, std::pair<int, int>>>
>;



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
            printf("Semaphore named %s\n",item.c_str() );
            sem_t *ss = sem_open(item.c_str(), O_CREAT | O_EXCL, 0666, 1);
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
        sem_t *ss = sem_open(name.c_str(), O_CREAT | O_EXCL, 0666, 0);
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
    MyPriorityQueue pq;




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


    int fd1[100][2], fd2[100][2], fd3[100][2], fd4[100][2], fd5[100][2], fd6[100][2]; 
    for(int i=0; i<process; i++){
        if(pipe(fd1[i]) == -1){
            printf("Error in making a pipe\n");
            fflush(stdout);
        }
        if(pipe(fd2[i]) == -1){
            printf("Error in making a pipe\n");
            fflush(stdout);
        }
        if(pipe(fd3[i]) == -1){
            printf("Error in making a pipe\n");
            fflush(stdout);
        }
        if(pipe(fd4[i]) == -1){
            printf("Error in making a pipe\n");
            fflush(stdout);
        }
        if(pipe(fd5[i]) == -1){
            printf("Error in making a pipe\n");
            fflush(stdout);
        }
        if(pipe(fd6[i]) == -1){
            printf("Error in making a pipe\n");
            fflush(stdout);
        }
    }



    // computation_time = number of request and release 
    //                    + parenthesized value in calculate + x from use_resources 




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



    // printf("\n");
    // printf("%d %d\n", ID, process);
    // fflush(stdout);
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
                if(idx != last_process){
                    sem_post(processSemaphore[idx]);
                    std::vector<int> b(resources) ;
                    read(fd1[idx][0], b.data(), sizeof(int)*resources);
                    printf("Received vector from child process%d:\n", idx);
                    for(int x: b){
                        printf("%d ", x);
                    }
                    printf("\n");
                    fflush(stdout);
                    int request_success=1;
                    write(fd5[idx][1], &request_success, sizeof(int));
                    main_process_in_bankers_algo=idx;
                    last_process=idx;
                    if(request_success){
                        int process_ended=0;
                        read(fd2[idx][0],  &process_ended, sizeof(int));
                        if(process_ended == 1){
                            main_process_in_bankers_algo=-1;
                            endcount++;
                        }
                        else{
                            int relative_time, computation;
                            read(fd3[idx][0],  &relative_time, sizeof(int));
                            read(fd4[idx][0],  &computation, sizeof(int));
                            pq.push({deadline[idx], {computation-computation_time[idx], idx}});
                        }
                    }
                    else{
                        printf("Case:1\n");
                        printf("Received request from child process%d: declined\n", idx);
                        fflush(stdout);
                        pq.push(current);
                    }
                }
                else{
                    auto current2=current;
                    current= pq.top();
                    pq.pop();
                    pq.push(current2);
                    idx =current.second.second;
                    sem_post(processSemaphore[idx]);
                    std::vector<int> b(resources) ;
                    read(fd1[idx][0], b.data(), sizeof(int)*resources);
                    printf("Received vector from child process%d:\n", idx);
                    for(int x: b){
                        printf("%d ", x);
                    }
                    printf("\n");
                    fflush(stdout);
                    int request_success=1;
                    for(int i =0; i<resources; i++){
                        if(b[i]>shared_numbers[i]){
                            request_success=0;
                            break;
                        }
                    }
                    write(fd5[idx][1], &request_success, sizeof(int));
                    main_process_in_bankers_algo=idx;
                    last_process=idx;
                    if(request_success){
                        int process_ended=0;
                        read(fd2[idx][0],  &process_ended, sizeof(int));
                        if(process_ended == 1){
                            main_process_in_bankers_algo=-1;
                            endcount++;
                        }
                        else{
                            int relative_time, computation;
                            read(fd3[idx][0],  &relative_time, sizeof(int));
                            read(fd4[idx][0],  &computation, sizeof(int));
                            pq.push({deadline[idx], {computation-computation_time[idx], idx}});
                        }
                    }
                    else{
                        printf("Case:2\n");
                        printf("Received request from child process%d: declined\n", idx);
                        fflush(stdout);
                        pq.push(current);
                    }
                }
            }
            else{
                if(main_process_in_bankers_algo != last_process){
                    MyPriorityQueue pq_temp1 =pq;
                    auto current = pq_temp1.top(); // Here main_process_temp != main_process
                    pq_temp1.pop();
                    while(!pq_temp1.empty()){
                        if(current.second.second == main_process_in_bankers_algo){
                            break;
                        }
                        else{
                            current = pq_temp1.top();
                            pq_temp1.pop();
                        }
                    }
                    MyPriorityQueue pq_temp2;
                    while (!pq.empty()){
                        auto temp = pq.top();
                        pq.pop();
                        if(temp != current){
                            pq_temp2.push(temp);
                        }
                    }
                    pq=pq_temp2;

                    int idx =current.second.second;
                    sem_post(processSemaphore[idx]);
                    std::vector<int> b(resources) ;
                    read(fd1[idx][0], b.data(), sizeof(int)*resources);
                    printf("Received vector from child process%d:\n", idx);
                    for(int x: b){
                        printf("%d ", x);
                    }
                    printf("\n");
                    fflush(stdout);
                    int request_success=1;
                    for(int i =0; i<resources && idx != main_process_in_bankers_algo && main_process_in_bankers_algo!=1; i++){
                        if(maximum[main_process_in_bankers_algo][i]>shared_numbers[i]- b[i]){
                            request_success=0;
                            break;
                        }
                    }
                    write(fd5[idx][1], &request_success, sizeof(int));
                    last_process=idx;
                    if(request_success){
                        int process_ended=0;
                        read(fd2[idx][0],  &process_ended, sizeof(int));
                        if(process_ended == 1){
                            main_process_in_bankers_algo=-1;
                            endcount++;
                        }
                        else{
                            int relative_time, computation;
                            read(fd3[idx][0],  &relative_time, sizeof(int));
                            read(fd4[idx][0],  &computation, sizeof(int));
                            pq.push({deadline[idx], {computation-computation_time[idx], idx}});
                        }
                    }
                    else{
                        printf("Case:3\n");
                        printf("Received request from child process%d: declined\n", idx);
                        fflush(stdout);
                        pq.push(current);
                    }
                }
                else{
                    auto current= pq.top();
                    pq.pop();
                    int idx =current.second.second;
                    if(idx == last_process){
                        auto current2= pq.top();
                        pq.pop();
                        pq.push(current);
                        current= current2;
                        idx =current2.second.second;
                    }
                    sem_post(processSemaphore[idx]);
                    std::vector<int> b(resources) ;
                    read(fd1[idx][0], b.data(), sizeof(int)*resources);
                    printf("Received vector from child process%d:\n", idx);
                    for(int x: b){
                        printf("%d ", x);
                    }
                    printf("\n");
                    fflush(stdout);
                    int request_success=1;
                    for(int i =0; i<resources && idx != main_process_in_bankers_algo && main_process_in_bankers_algo!=1; i++){
                        if(maximum[main_process_in_bankers_algo][i]>shared_numbers[i]- b[i]){
                            request_success=0;
                            break;
                        }
                    }
                    write(fd5[idx][1], &request_success, sizeof(int));
                    last_process=idx;
                    if(request_success){
                        int process_ended=0;
                        read(fd2[idx][0],  &process_ended, sizeof(int));
                        if(process_ended == 1){
                            endcount++;
                        }
                        else{
                            int relative_time, computation;
                            read(fd3[idx][0],  &relative_time, sizeof(int));
                            read(fd4[idx][0],  &computation, sizeof(int));
                            pq.push({deadline[idx], {computation-computation_time[idx], idx}});
                        }
                    }
                    else{
                        printf("Case:4\n");
                        printf("Received request from child process%d: declined\n", idx);
                        fflush(stdout);
                        pq.push(current);
                    }
                }
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
        for(int i=0; i<resources; i++){
            std::vector<sem_t *> t;
            master_sem_t.push_back(t);
        }
        int relative_time=0;
        int computationTime1=0;
        int first_request =0;
        std::vector<int> allocated(resources, 0);
        while(current_instruction < processInstructions[ID].Ins.size()){
            printf("Process%d LOOP\n", ID);
            fflush(stdout);
            if(processInstructions[ID].Ins[current_instruction].first == 1){
                relative_time++;
                if(first_request != 0){
                    int process_ended=0;
                    write(fd2[ID][1],  &process_ended, sizeof(int));
                    write(fd3[ID][1],  &relative_time, sizeof(int));
                    write(fd4[ID][1],  &computationTime1, sizeof(int));
                }
                first_request++;
                sem_wait(processSemaphore[ID]); 
                std::vector<int> a = processInstructions[ID].Ins[current_instruction].second;
                printf("The request vector from %d is:\n", ID);
                for(auto x: a){
                    printf("%d ", x);
                }
                printf("\n");
                fflush(stdout);
                write(fd1[ID][1], a.data(), sizeof(int)*resources);
                int request_success=0;
                read(fd5[ID][0], &request_success, sizeof(int));
                if(request_success){
                    computationTime1++;
                    for(int i=0; i<resources; i++){
                        shared_numbers[i]-=a[i];
                        allocated[i]+=a[i];
                    }
                    for(int i=0; i<resources; i++){
                        int accquire_num = processInstructions[ID].Ins[current_instruction].second[i];
                        int j=0;
                        while(accquire_num>0){
                            int sval;
                            sem_t *ss=resourceListSemaphore[i][j];
                            sem_getvalue(ss, &sval);
                            printf("%d %d %d %d the sval is %d\n", ID, i, accquire_num, j, sval);
                            if(sval>0){
                                sem_wait(ss);
                                master_sem_t[i].push_back(ss);
                                master_string[i].push_back(resourceList[i][j]);
                                j++;
                                accquire_num--;
                            }
                            else{
                                j++;
                            }
                        }
                    }
                    printf("request is success\n");
                    fflush(stdout);
                    // Note: one semaphore might be needed here
                }
                else{
                    first_request--;
                    current_instruction--;
                }
            }
            else if(processInstructions[ID].Ins[current_instruction].first == 2){
                //release
                relative_time++;
                computationTime1++;
                for(int i=0; i<resources; i++){
                    int release_num = processInstructions[ID].Ins[current_instruction].second[i];
                    for(int j=0; j<release_num; j++){
                        sem_t * ss = master_sem_t[i].back();
                        sem_post(ss);
                        master_sem_t[i].pop_back();
                        master_string[i].pop_back();
                    }
                    shared_numbers[i] += release_num;
                    allocated[i]      -= release_num;
                }
            }
            else if(processInstructions[ID].Ins[current_instruction].first == 3){
                //calculate
                computationTime1+=processInstructions[ID].Ins[current_instruction].second[0];
            }
            else if(processInstructions[ID].Ins[current_instruction].first == 4){
                //use_resorusces
                computationTime1+=processInstructions[ID].Ins[current_instruction].second[0];
            }
            else if(processInstructions[ID].Ins[current_instruction].first == 5){
                printf("The master_string is \n");
                for(auto x: master_string){
                    for(auto y: x){
                        printf("%s, ", y.c_str());
                    }
                }
                printf("\n");
                fflush(stdout);
                //print_recouses_used
            }
            else{
                ;
            }
            current_instruction++;
        }

        for(int i=0; i<resources; i++){
            shared_numbers[i] +=  allocated[i];
        }
        printf("Shared Numbers are:\n");
        for(int i=0; i<resources; i++){
            printf("%d ", shared_numbers[i]);
        }
        printf("\n");
        fflush(stdout);
        for(int i=0; i<resources; i++){
            int size = master_sem_t[i].size();
            for(int j=0; j<size; j++){
                sem_t * ss = master_sem_t[i].back();
                sem_post(ss);
                master_sem_t[i].pop_back();
                master_string[i].pop_back();
            }
        }
        printf("Process%d ENDED\n", ID);
        fflush(stdout);
        if(relative_time>deadline[ID]){
            printf("Process%d missed with deadline\n", ID);
            fflush(stdout);
        }
        int process_ended=1;
        write(fd2[ID][1],  &process_ended, sizeof(int));
        printf("The svals after ending process is\n");
        for(auto x: resourceListSemaphore){
            for(auto y: x){
                int sval=9000;  //Random value for debug purpose
                sem_getvalue(y, &sval);
                printf("%d ", sval);
            }
            printf("\n");
            fflush(stdout);
        }
    }


    


    while (wait(NULL) != -1){
        ;
    }
    munmap(shared_numbers, sizeof(int) * resources);
    shm_unlink(SHARED_MEMORY_NAME);


    return 0;
}