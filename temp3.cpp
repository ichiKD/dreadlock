

// INS[i] = 
// request(1, 2, 3, 4, .. m) = {1, {1, 2, 3, ... m}}
// release(1, 2, 3, 4, .. m) = {2, {1, 2, 3, ... m}}
// calculate(x) = {3, {x}}
// use_resorusces(x, y) = {4, {x, y}}
// print_recouses_used = {5, {}}
// end = {6, {}}

int resources, process;
int maximum[100][100];
int avaliable[100];
int deadline[100];
int computation_time[100];
bool done[100];
int processEnded;
std::vector<struct Instructions> processInstructions; 
std::vector<int> index_of_last_request_yet_to_be_processed;
std::vector< std::vector<std::string> > resourceList;
std::vector< std::vector< std::pair< sem_t, std::string> > > resourceListSemaphore;
std::vector<sem_t> processSemaphore;

struct Instructions{
std::vector<std::pair<int, std::vector<int>>> Ins;

};

int ID = process;
pid_t pid;
