last_process = select_process=1

select_process2 =1
send_yes_to_process=1
send_no_to_process=1
pq=1
endcount=0

receive_end_process=1
receive_deadline_computation_time=1
deadline=1
computation_time=1



send_deadline_computation_time=1
send_not_ended=0
send_ended=1
do_operation=receive_approval=acquire_resources=1

main_process_in_bankers_algo=1


def sem_post():
    print()
def sem_wait():
    print()

def receive_vector():
    print()
def check_banker_algo():
    print()

def Scheduler():
    if(main_process_in_bankers_algo == -1):
        select_process
        if(select_process == last_process):
            select_process2
            pq.push(select_process)
            last_process=select_process2.id
            sem_post(select_process2)
            r=receive_vector
            check=check_banker_algo()
            if(check):
                send_yes_to_process()
                receive_end_process()
                if(receive_end_process):
                    endcount+=1
                    main_process_in_bankers_algo=-1
                else:
                    receive_deadline_computation_time()
                    pq.push(deadline, computation_time, select_process2)
            else:
                send_no_to_process()
                pq.push(select_process2)
        else:
            last_process=select_process.id
            sem_post(select_process)
            r=receive_vector
            check=check_banker_algo()
            if(check):
                send_yes_to_process()
                receive_end_process()
                if(receive_end_process):
                    endcount+=1
                else:
                    receive_deadline_computation_time()
                    pq.push(deadline, computation_time, select_process)
            else:
                send_no_to_process()
                pq.push(select_process)
    else:
        if(last_process != main_process_in_bankers_algo):
            last_process = main_process_in_bankers_algo
            sem_post(main_process_in_bankers_algo)
            r=receive_vector
            check=check_banker_algo()
            if(check):
                send_yes_to_process()
                receive_end_process()
                if(receive_end_process):
                    endcount+=1
                    main_process_in_bankers_algo=-1
                else:
                    receive_deadline_computation_time()
                    pq.push(deadline, computation_time, main_process_in_bankers_algo)
            else:
                send_no_to_process()
                pq.push(main_process_in_bankers_algo)
        else:
            last_process=select_process.id
            sem_post(select_process)
            r=receive_vector
            check=check_banker_algo()
            if(check):
                send_yes_to_process()
                receive_end_process()
                if(receive_end_process):
                    endcount+=1
                else:
                    receive_deadline_computation_time()
                    pq.push(deadline, computation_time, select_process)
            else:
                send_no_to_process()
                pq.push(select_process)

def process():
    num_instructions=10
    ins=[]
    for i in range(num_instructions):
        if(ins[i] == "request"):
            send_not_ended
            send_deadline_computation_time
            sem_wait()
            receive_approval
            if(receive_approval):
                acquire_resources
            else:
                i-=1
        elif(ins[i] == "end."):
            send_ended
        else:
            do_operation()


# struct process{
#     int id;
#     vector<vector<int>> index_to_accquired_resources;
# };
# vector<struct Instructions> processInstructions; 
# vector<int> index_of_last_request_yet_to_be_processed;
# vector<vector<string> > resourceList;
# vector<vector<sem_t *>> resourceListSemaphore;
# vector<sem_t> processSemaphore;













