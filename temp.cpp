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

using namespace std;
using  ns = chrono::nanoseconds;
using get_time = chrono::steady_clock;

/*
        I suppose some of the features are not as
*/

// Variables
int m = 0;                                              //      The total number of resources
int n = 0;                                              //      The total number of processes
struct Resource                                                 //      Structure of the resource node
{
        int ID;
        int available;
};
Resource *resources;                                    //      Array that contains all the structures of the resrouce nodes
struct Process
{
        int ID;
        int deadline;
        int computeTime;                                        //      Integer equal to number of requests and releases plus the parenthesized values in the calculate and use_resources instructions.
        int *allocatedResources;                        //      The amount of resources that is currently allocated to the process per resource
        int *maxResources;                                      //      Array representing the max amount of resources a process need to complete process from each resource
        int *neededResources;                           //      Array of how much resources needed left to complete execution per resouce
        string *instructions;                           //      Array of instructions for the processor
        int pipe_ParentWriteToChild[2];
        int pipe_ChildWriteToParent[2];
        vector<int> allocated; 
        vector<int> max;
        int laxity;
        string masterString; 
};
Process *processes;                                             //      Array that contains all the structures of the processes
char buffer[1000];                                              //      Character buffer length of write message
int bufferLength;
int instructionsToProcess = 0;
int numOfProcessesRemaining;                    //      The number of processes remaining to process.

//      Methods
void ReadFromFile(string inputFileName);
int GetFirstIntInString(string inputString);
int GetMaxResourcePerProcessorValue(string inputString);
void SortProcessesByDeadline(std::vector<Process>& processes, int left, int right);
void CreatePipesForProcesses();
void EvaluateMessage(Process process, string message);
bool Safe();
void calculate(string message, Process process, int computeTime);
void request(string message, Process process, int requestInts[]);
void release(string message, Process process, int releaseInts[]);
void use_resources(string message, Process process, int amount);
// Function declarations
void EDF(vector<Process> processes, vector<int> available);
void LLF(vector<Process> processes, vector<int> available); 


// Data structures to hold details about processes

// Print functions 
void printAvailable(vector<int> available) {
  cout << "available = [";
  for(int i = 0; i < available.size(); i++) {
    cout << available[i];
    if(i < available.size() - 1) {
      cout << ", ";
    }
  }
  cout << "]" << endl;
}

void printAllocation(vector<Process> processes) {
  cout << "allocation = ";
  for(int i = 0; i < processes.size(); i++) {
    cout << "[";
    for(int j = 0; j < processes[i].allocated.size(); j++) {
      cout << processes[i].allocated[j];
      if(j < processes[i].allocated.size() - 1) {
        cout << ", ";
      }
    }
    cout << "]";
    if(i < processes.size() - 1) {
      cout << ", ";
    }
  }
  cout << endl;
}

void printNeed(vector<Process> processes) {
  cout << "need = ";
  // Print need matrix
}

void printDeadline(vector<Process> processes) {
  cout << "deadline = [";
  for(int i = 0; i < processes.size(); i++) {
    cout << processes[i].deadline;
    if(i < processes.size() - 1) {
      cout << ", "; 
    }
  }
  cout << "]" << endl;
}

void printLaxity(vector<Process> processes) {
  cout << "laxity = [";
  for(int i = 0; i < processes.size(); i++) {
    cout << processes[i].laxity;
    if(i < processes.size() - 1) {
      cout << ", ";
    }
  }
  cout << "]" << endl; 
}

void printMasterString(Process process) {
  cout << "Process " << process.ID << " master string: " << process.masterString << endl;
}


int main(int argc, char* argv[])
{
        //      Start compute timer
        auto start = get_time::now();

        //      Read, Evaluate, and Assign variables based in the input .txt file supplied by command argument
        ReadFromFile(argv[1]);
        // Initialize processes and resources
        vector<Process> processes;
        vector<int> available;
        //      Sort processes by deadline and computation time by Longest Job First - LJF
        SortProcessesByDeadline(processes, 0, n-1);
        cout << "Sorting sequence of process execution by lowest deadline first & highest computation time..." << endl;
        for (int i = 0; i < n; i++)
                cout << " " << i+1 << ") Process " << processes[i].ID << " with deadline: " << processes[i].deadline << " and computation time: " << processes[i].computeTime << endl;

        cout << endl;   //      Skip a line for neatness

        //      Initialize pipes for parent-child communication
        CreatePipesForProcesses();

        cout << endl;   //      Skip a line for neatness

        //      Create each process with fork() evaluate its id
        int processID;
        int mainParentProcessID = getpid();

        //      Create child fork() processes & assign a Process variable
        Process currentProcess;
        for (int i = 0; i < n; i++)
        {
                currentProcess = processes[i];

                processID = fork();

                if (getpid() != mainParentProcessID)
                        break;
        }

        //      Process failed to create
        if (processID == -1)
        {
                perror("ERROR: unable to create process.");
                exit(0);
        }
        //      Process is a CHILD
        else if (processID == 0)
        {
                //cout << endl; //      Skip a line for neatness
                cout << "Forked Child Process: " << currentProcess.ID << endl << endl;

                //      Loop through each instruction and process it to Main Process
                for (int i = 0; i < sizeof(currentProcess.instructions); i++)
                {
                        //close(currentProcess.pipe_ChildWriteToParent[0]);
                        write(currentProcess.pipe_ChildWriteToParent[1], currentProcess.instructions[i].c_str(), bufferLength);

                        cout << "Process " << currentProcess.ID << " sent instruction: " << currentProcess.instructions[i] << endl;

                        //close(currentProcess.pipe_ParentWriteToChild[1]);
                        while (true)
                        {
                                read(currentProcess.pipe_ParentWriteToChild[0], buffer, bufferLength);

                                string instructionFeedBack = buffer;

                                if (instructionFeedBack.find("SUCCESS") != string::npos)
                                {
                                        cout << "Process " << currentProcess.ID << " completed instruction: " << instructionFeedBack << endl << endl;
                                        //instructionsLeft--;
                                        break;
                                }
                                else if (instructionFeedBack.find("TERMINATE") != string::npos)
                                {
                                        cout << "Process " << currentProcess.ID << " terminated." << endl;      //Skip a line for neatness

                                        write(currentProcess.pipe_ChildWriteToParent[1], "TERMINATED", bufferLength);

                                        exit(0);

                //instructionsLeft--;
                                }
                                else if (instructionFeedBack.find("WAIT") != string::npos)
                                {
                                        //      Process waits...
                                        //write(currentProcess.pipe_ChildWriteToParent[1], currentProcess.instructions[i].c_str(), bufferLength);

                                        cout << "Process " << currentProcess.ID << " attempting to send instruction: " << currentProcess.instructions[i] << " again." << endl;

                                }
                                else
                                        cout << "Process " << currentProcess.ID << " is listening..." << endl;
                        }

                        //      Close the Read end
                        //close(currentProcess.pipe_ParentWriteToChild[0]);
                }

                //      Send termination message
                write(currentProcess.pipe_ChildWriteToParent[1], "TERMINATED", bufferLength);

                cout << "Process " << currentProcess.ID << " has no more instructions. Process terminated." << endl;
//      Skip a line for neatness

                exit(0);
        }
        //      Process if a PARENT/Main Process
        else if(getpid() == mainParentProcessID)
        {
                numOfProcessesRemaining = n;
                while(numOfProcessesRemaining > 0)
                {
                        //string message = ReadFromPipe(processes[0]);

                        for (int i = 0; i < n; i++)
                        {
                                //close(processes[0].pipe_ChildWriteToParent[1]);
                                read(processes[i].pipe_ChildWriteToParent[0], buffer, bufferLength);

                                string instructionMessage = buffer;

                                //      if the read buffer is not empty... evaluate message
                                if (sizeof(instructionMessage) > 0)
                                        EvaluateMessage(processes[i], instructionMessage);
                        }
                }
                //close(processes[0].pipe_ChildWriteToParent[0]);
        }

        // Schedule using EDF
        //EDF(processes, available);

        // Print newline  
        cout << endl;

        // Schedule using LLF 
        //LLF(processes, available);
        cout << "\nNo more instructions left to process. Main Process terminating..." << endl;

        //      End clock and display the compute time
        auto end = get_time::now();
        auto diff = end - start;
        cout << "\nTotal compute time: " << chrono::duration_cast<std::chrono::milliseconds>(diff).count() << " milliseconds. " << endl << endl;

        return 0;
}

//#pragma region ReadFromFile(): Read, Evaluate, and Assign variables based in the input .txt file
void ReadFromFile(string inputFileName)
{
        //      input the string argument of the input file
        fstream inputFile(inputFileName);

        //      Evaluate the input file
        if (inputFile.is_open())
        {
                cout << "\n" << "Opened file: " << inputFileName << "\n\n";

                //      Get the first line
                string currentLine;

                //      Find & Assign the amount of resources
                getline(inputFile, currentLine);
                m = GetFirstIntInString(currentLine);

                //      Initialize size of resources array & avaliable array
                resources = new Resource[m];
                //available = new int[m];
                cout << "Total Resources: " << m << endl;

                //      Find & Assign the amount of processes
                getline(inputFile, currentLine);
                n = GetFirstIntInString(currentLine);

                //      Initialize size of processes array and pipe read/write size which is 2
                processes = new Process[n];
                //for (int i = 0; i < n; i++)
                        //processes[i] = new Process[2];
                cout << "Total Processes: " << n << "\n\n";

                //      Determine the ID and amount of resources each resource has
                for (int i = 0; i < m; i++)
                {
                        getline(inputFile, currentLine);

                        //      Create new resource struct and add it to array of resources
                        Resource resource;
                        resources[i] = resource;
                        resources[i].ID = i;
                        resources[i].available = GetFirstIntInString(currentLine);
                        //available[i] = resources[i].amount;
                        cout << "Resource " << resources[i].ID + 1 << " has " << resources[i].available << " amount of resources." << endl;
                }

                cout << endl;   //      Skip a line for neatness

                //      Find & Assign the size of resource related parameters for the process
                for (int i = 0; i < n; i++)
                {
                        processes[i].allocatedResources = new int[m];
                        processes[i].maxResources = new int[m];
                        processes[i].neededResources = new int[m];
                }
                //maxResourcePerProcess = new int*[n];
                //for (int i = 0; i < n * m; i++)
                //      maxResourcePerProcess[i] = new int[m];

                //      Loop through the each process and assign the value of the max value processor can demand from each resource
                for (int i = 0; i < n; i++)
                {
                        cout << "Max resources Process " << i + 1 << " that can demand from:" << endl;

                        for (int j = 0; j < m; j++)
                        {
                                //      Get new line and find value in string
                                getline(inputFile, currentLine);

                                processes[i].maxResources[j] = GetMaxResourcePerProcessorValue(currentLine);
                                processes[i].neededResources[j] = processes[i].maxResources[j];

                                //      Display result
                                cout << " Resource " << j + 1 << ": " << processes[i].maxResources[j] << endl;
                                //cout << " Resource " << j + 1 << ": " << maxResourcePerProcess[i][j] << endl;
                        }
                }

                cout << endl;   //      Skip a line for neatness

                //      Loop through each process and cache their parameters
                for (int i = 0; i < n; i++)
                {
                        //      Skip all lines until next process
                        while (true)
                        {
                                getline(inputFile, currentLine);
                                if (currentLine.find("process_") != string::npos)
                                        break;
                        }

                        cout << "Fetching parameters for " << currentLine << "..." << endl;

                        //      ID
                        processes[i].ID = i + 1;

                        //      Deadline
                        getline(inputFile, currentLine);
                        processes[i].deadline = GetFirstIntInString(currentLine);
                        cout << "Process " << i+1 << " deadline: " << processes[i].deadline << endl;

                        //      Compute time
                        getline(inputFile, currentLine);
                        processes[i].computeTime = GetFirstIntInString(currentLine);
                        cout << "Process " << i+1 << " compute time: " << processes[i].computeTime << endl;

                        //      Calculate & Assign the amount of instructions for this process
                        int instructionAmount = 0;
                        streampos originalPos = inputFile.tellg();              //      Cache line position
                        while (true)
                        {
                                getline(inputFile, currentLine);

                                //      Break loop if a "end" line is found & assign the length of instructions array
                                if (currentLine.find("end") != string::npos)
                                {
                                        processes[i].instructions = new string[instructionAmount];
                                        inputFile.seekg(originalPos, ios::beg);                 //      Set the getline back to the original position
                                        break;
                                }
                                instructionAmount++;
                        }

                        cout << "Process " << i+1 << " instructions:" << endl;

                        //      Loop through instructions and cache them into process string array
                        for (int j = 0; j < instructionAmount; j++)
                        {
                                getline(inputFile, currentLine);
                                processes[i].instructions[j] = currentLine;

                                //      increment the total amount of instructions
                                instructionsToProcess++;

                                cout << " " << j+1 << ") " << processes[i].instructions[j] << endl;
                        }

                        cout << endl;   //      Skip a line for neatness
                }

                inputFile.close();
        }
        else
        {
                cout << "ERROR: invalid file input or file not found." << endl;
                exit(0);
        }
}
//#pragma endregion

//#pragma region GetFirstIntInString(): Returns the first integer in the given string
int GetFirstIntInString(string inputString)
{
        //      Loop through the string until the next char is not a number
        /*int i;
        for (i = 0; i < inputString.length(); i++)
                if (inputString[i] < '0' || inputString[i] >= '9')
                        break;

        //      Convert the number string into an integer using string length of i
        string numberString;
        for (int j = 0; j < i; j++)
                numberString += inputString[j];

        return stoi(numberString);*/

        return stoi(inputString);                       //      Updated input files no longer has comments, thus no need to do line parsing evaluation
}
//#pragma endregion

//#pragma region GetMaxResourcePerProcessorValue(): Returns the integer of the max value that the processor can demand from each resource node
int GetMaxResourcePerProcessorValue(string inputString)
{
        //      Find the first '=' sign which indicates that the desired value is after it
        int pos = inputString.find("=");

        //      Create a sub string for everything after the '=' sign
        string intSubString = inputString.substr(pos + 1);

        return stoi(intSubString);
}
//#pragma endregion

//#pragma region SortProcessesByDeadline():  Sort processes by deadline using Quick Sort Algorithm. If tied, the longer compute time takes prioity
void SortProcessesByDeadline(std::vector<Process>& processes, int left, int right)
{
    int i = left, j = right;
    Process temp;
    Process pivot = processes[(left + right) / 2];
    // Partioning
    while (i <= j)
    {
        while (processes[i].deadline < pivot.deadline)
            i++;
        while (processes[j].deadline > pivot.deadline)
            j--;
        while (processes[j].deadline == pivot.deadline && processes[j].computeTime < pivot.computeTime)
            j--;
        if (i <= j)
        {
            temp = processes[i];
            processes[i] = processes[j];
            processes[j] = temp;
            i++;
            j--;
        }
    }
    // Recursion
    if (left < j)
        SortProcessesByDeadline(processes, left, j);
    if (i < right)
        SortProcessesByDeadline(processes, i, right);
}



//#pragma endregion

//#pragma region CreatePipesForProcesses(): Creates a pipe for each process
void CreatePipesForProcesses()
{
        cout << "Creating pipes for Processes..." << endl;
        for (int i = 0; i < n; i++)
        {
                //      If process failed to create a pipe... exit
                if (pipe(processes[i].pipe_ChildWriteToParent) == -1)
                {
                        perror("ERROR: unable to create Child to Parent pipe.");
                        exit(0);
                }
                if (pipe(processes[i].pipe_ParentWriteToChild) == -1)
                {
                        perror("ERROR: unable to create Parent to Child pipe.");
                        exit(0);
                }

                cout << " Pipe created for Process " << processes[i].ID << endl;
        }

        //      Initialize buffer size of char array for pipe
        bufferLength = sizeof(buffer) / sizeof(buffer[0]);
        cout << endl << "Character buffer length for pipes: " << bufferLength << endl;
}
//#pragma endregion

//#pragma region EvaluateMessage():  Evaluate the message and run their respective methods
void EvaluateMessage(Process process, string message)
{
        if (message.find("calculate") != string::npos)
        {
                cout << "Main Process received instruction: " << message << " from Process " << process.ID << endl;

                calculate(message, process, 1);
        }
        else if (message.find("request") != string::npos)
        {
                cout << "Main Process received instruction: " << message << " from Process " << process.ID << endl;

                //      Find all integers in the message and store it as an array
                int intsInMessage[m];
                int intIndexInString = 0;
                string number;
                for (int i = 0; i < message.length(); i++)
                {
                        if (isdigit(message[i]))
                        {
                                //found a digit, get the int
                                for (int j = i; ; j++)
                                {
                                        if (isdigit(message[j]))                //consecutive digits
                                                number += message[j];
                                        else
                                        {
                                                i = j - 1;              //set i to the index of the last digit
                                                break;
                                        }
                                }
                                intsInMessage[intIndexInString] = atoi(number.c_str());
                                number = "";
                                intIndexInString++;
                        }
                }


                //      Perform request function and pass the parameter
                request(message, process, intsInMessage);
        }
        else if (message.find("release") != string::npos)
        {
                cout << "Main Process received instruction: " << message << " from Process " << process.ID << endl;

                //      Find all integers in the message and store it as an array
                int intsInMessage[m];
                int intIndexInString = 0;
                string number;
                for (int i = 0; i < message.length(); i++)
                {
                        if (isdigit(message[i]))
                        {       //found a digit, get the int
                                for (int j = i; ; j++)
                                {
                                        if (isdigit(message[j]))                //consecutive digits
                                                number += message[j];
                                        else
                                        {
                                                i = j - 1;              //set i to the index of the last digit
                                                break;
                                        }
                                }
                                intsInMessage[intIndexInString] = atoi(number.c_str());
                                number = "";
                                intIndexInString++;
                        }
                }

                //      Perform release function and pass the parameter
                release(message, process, intsInMessage);
        }
        else if (message.find("use_resources") != string::npos)
        {
                cout << "Main Process received instruction: " << message << " from Process " << process.ID << endl;

                use_resources(message, process, 1);
        }
        //      if process sent termination message... tell main process that it has been terminated
        else if (message.find("TERMINATED") != string::npos)
        {
                numOfProcessesRemaining--;
        }
        else if(message.find("print_resources_used") != string::npos){

        }
        //      else, message is invalid... Terminate the child process
        else
        {
                write(process.pipe_ChildWriteToParent[1], "TERMINATE", bufferLength);

                cout << "ERROR: invalid instruction message." << endl;
                exit(0);
        }
}
//#pragma endregion

//#pragma region Safe():  Checks if bankers algorithm is safe to continue
bool Safe()
{
        bool isSafe = true;

        return isSafe;
}
//#pragma endregion

//#pragma region calculate():  calculate without using resources. wait?
void calculate(string message, Process process, int computeTime)
{
        cout << "Calculating (" << computeTime << ") for Process " << process.ID << "..." << endl;

        cout << message << " instruction complete message written to Process " << process.ID << endl;

        message += "=SUCCESS";

        //close(process.pipe_ParentWriteToChild[0]);
        write(process.pipe_ParentWriteToChild[1], message.c_str(), bufferLength);
}
//#pragma endregion

//#pragma region request():  request vector, m integers
void request(string message, Process process, int requestInts[])
{
        //      Create string for request ints and display it
        string requestValues = "(";
        for (int i = 0; i < m; i++)
        {
                requestValues += to_string(requestInts[i]);
                if (i < m - 1)
                        requestValues += ",";
        }
        requestValues += ")";

        cout << "Requesting " << requestValues << " resources for Process: " << process.ID << "..." << endl;

        //      Cache original resource incase of failure
        int tempResourceArray[m];
        //      Copies the array to the temp array
        memcpy(tempResourceArray, process.allocatedResources, m);

        //      Perform Banker's Algorithm for deadlock avoidance for each resource request
         //     First perform first 2 checks for amount allocations.
        for (int i = 0; i < m; i++)
        {
                //      if the requested resources is higher then the need...
                if (requestInts[i] > process.neededResources[i])
                {
                        //      Reset the allocatedResources to their original values
                        process.allocatedResources = tempResourceArray;

                        cout << "Process " << process.ID << " is requesting more resources than it needs from Resource " << i+1 << ". Process is terminated"  << endl;

                        //      Send termination message
                        write(process.pipe_ChildWriteToParent[1], "TERMINATE", bufferLength);
                        return;
                }
                //      if the request is more then the available...
                else if (requestInts[i] > resources[i].available)
                {
                        //      Process waits
                        cout << "Process " << process.ID << " is requesting more resources than available. Process waits." << endl;

                        //      Send wait message
                        //write(process.pipe_ParentWriteToChild[1], "WAIT", bufferLength);

                        //return;
                        break;
                }
        }

        //      No errors in allocation, begin the actual allocation.
        for (int i = 0; i < m; i++)
        {
                resources[i].available -= requestInts[i];
                process.allocatedResources[i] += requestInts[i];
                process.neededResources -= requestInts[i];
        }

        //      Create a string of the array amount of allocated resources in process
        string allocatedValues = "(";
        for (int i = 0; i < m; i++)
        {
                allocatedValues += to_string(process.allocatedResources[i]);
                if (i < m - 1)
                        allocatedValues += ",";
        }
        allocatedValues += ")";

        cout << "Process " << process.ID << " now has " << allocatedValues << " allocated resources." << endl;

        //      Display amount of availiable resources in Resource
        for (int i = 0; i < m; i++)
                cout << "Resource " << resources[i].ID << " now has " << resources[i].available << " availiable resources." << endl;

        //      Check for Safe
        if (Safe())
        {
                //      Complete transaction
                cout << "Process " << process.ID << " is safe. Transaction completed." << endl;

        }
        else
        {
                //      Process must wait
                cout << "Process " << process.ID << " is not safe. Process is waiting." << endl;
                return;
        }

        cout << message << " instruction complete message written to Process " << process.ID << endl;

        message += "=SUCCESS";

        //close(process.pipe_ParentWriteToChild[0]);
        write(process.pipe_ParentWriteToChild[1], message.c_str(), bufferLength);

}
//#pragma endregion

//#pragma region release():  release vector, m integers
void release(string message, Process process, int releaseInts[])
{
        //      Create string for release ints and display it
        string releaseValues = "(";
        for (int i = 0; i < m; i++)
        {
                releaseValues += to_string(releaseInts[i]);
                if (i < m - 1)
                        releaseValues += ",";
        }
        releaseValues += ")";

        cout << "Releasing " << releaseValues << " resources for Process: " << process.ID << "..." << endl;

        //      Loop through each release amount and adjust
        for (int i = 0; i < m; i++)
        {
                resources[i].available += releaseInts[i];
                process.allocatedResources[i] -= releaseInts[i];
                process.neededResources += releaseInts[i];
        }

        //      Create a string of the array amount of allocated resources in process
        string allocatedValues = "(";
        for (int i = 0; i < m; i++)
        {
                allocatedValues += to_string(process.allocatedResources[i]);
                if (i < m - 1)
                        allocatedValues += ",";
        }
        allocatedValues += ")";

        cout << "Process " << process.ID << " now has " << allocatedValues << " allocated resources." << endl;

        //      Display amount of availiable resources in Resource
        for (int i = 0; i < m; i++)
                cout << "Resource " << resources[i].ID << " now has " << resources[i].available << " availiable resources." << endl;

        cout << message << " instruction complete message written to Process " << process.ID << endl;
        message += "=SUCCESS";

        //close(process.pipe_ParentWriteToChild[0]);
        write(process.pipe_ParentWriteToChild[1], message.c_str(), bufferLength);
}
//#pragma endregion

//#pragma region use_resources():  use allocated resources
void use_resources(string message, Process process, int amount)
{
        cout << "Using resources for Process: " << process.ID << "..." << endl;

        cout << "Process " << process.ID << " used (" << amount << ") allocated resources." << endl;

        cout << message << " instruction complete message written to Process " << process.ID << endl;
        message += "=SUCCESS";

        write(process.pipe_ParentWriteToChild[1], message.c_str(), bufferLength);
}
//#pragma endregion

// Scheduling algorithms
//#pragma region EDF():  use allocated resources
void EDF(vector<Process> processes, vector<int> available) {

  // Scheduling logic
  
  cout << "EDF Scheduler:" << endl;
  
  // Initialize
  printAvailable(available);
  printAllocation(processes);
  printNeed(processes);

  // Scheduling logic
  // ...

  // Print results
  cout << "EDF Scheduler Deadline Misses: " << endl;

}
//#pragma endregion


//#pragma region LLF():  use allocated resources
void LLF(vector<Process> processes, vector<int> available) {

  cout << "LLF Scheduler:" << endl;

  // Initialize
  printAvailable(available);
  printAllocation(processes);
  printNeed(processes);
  printDeadline(processes);
  printLaxity(processes);

  // Scheduling logic
  // ...
  
  // Print results
  cout << "LLF Scheduler Deadline Misses: " << endl;  
}

//#pragma endregion