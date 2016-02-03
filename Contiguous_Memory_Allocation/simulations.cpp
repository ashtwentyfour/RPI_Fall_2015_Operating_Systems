#include <algorithm>
#include "process_simulation.h"


Process* SRT_processes;

// compare burst times
bool lower_burst_time(int x, int y) {
  return SRT_processes[x-1].getCPUTime() < SRT_processes[y-1].getCPUTime();
}

// allocate memory and perform defragmentation if required
int Simulation::allocate(Process* Processes, long int& time,
                std::list<int>& queue, std::string alg) {
    // amt of time spent on defragmentation
    int time_change;
    time_change = 0;
    // queue used to decide the order in which processes are add (SRT or FCFS/RR)
    std::list<int> temp_queue;
    if(alg == "srt") { // for SRT add by priority
        for(int i = 0; i < n; i++)
            if(Processes[i].getArrivalTime() == time)
                temp_queue.push_back(i+1);
        temp_queue.sort(lower_burst_time);
        if(temp_queue.size() == 0)
            return 0;
    }
    else { // else assume round robin and add FCFS
        for(std::list<std::string>::iterator it = fcfs_order.begin(); it != fcfs_order.end(); it++) {
            for(int i = 0; i < n; i++) {
                if(Processes[i].getID() == *it && Processes[i].getArrivalTime() == time)
                    temp_queue.push_back(Processes[i].getProcNumber());
            }
        }
        if(temp_queue.size() == 0)
            return 0;
    }
    // add each process whose arrival time = t_current
    for(std::list<int>::iterator itr = temp_queue.begin(); itr != temp_queue.end(); itr++) {
        if(Processes[*itr-1].getArrivalTime() == time) {
            // get process ID
            char ID = (Processes[*itr-1].getID())[0];
            bool defragmented = false;
            // number of memory units moved
            int num_moved = 0;
            // process memory
            int mem = Processes[*itr-1].getMemory();
            fit->allocate_memory(ID, mem);
            // if defragmentation is needed
            if(fit->defragReq()) {
                defragmented = true;
                std::cout<<"time "<<time<<"ms: Process '"<<Processes[*itr-1].getID()<<"' unable to be added; lack of memory;";
                std::cout<<std::endl;
                std::cout<<"time "<<time<<"ms: Starting defragmentation (suspending all processes)";
                std::cout<<std::endl;
                std::cout<<"time "<<time<<"ms: Simulated Memory:";
                std::cout<<std::endl;
                fit->print();
                fit->defragment(num_moved);
                // try defragmentation and allocate again
                time += t_memmove*num_moved;
                time_change += t_memmove*num_moved;
                std::cout<<"time "<<time<<"ms: Completed defragmentation (moved "<<num_moved<<" units of memory)";
                std::cout<<std::endl;
                std::cout<<"time "<<time<<"ms: Simulated Memory:";
                std::cout<<std::endl;
                fit->print();
                fit->allocate_memory(ID, mem);
            }
            // if the defragmentation was successful - add the process
            if(!fit->defragReq() && defragmented == true) {
                std::cout<<"time "<<time<<"ms: "<<"Process '"<<Processes[*itr-1].getID()<<"' added to the system ";
                std::cout<<"[Q";
                queue.push_back(*itr);
                if(alg == "srt")
                    queue.sort(lower_burst_time);
                for(std::list<int>::iterator it = queue.begin(); it != queue.end(); it++)
                    std::cout<<" "<<Processes[*it-1].getID();
                std::cout<<"]";
                std::cout<<std::endl;
            }
            // if unsuccessful skip the process
            else if(fit->defragReq() && defragmented) {
                std::cout<<"time "<<time<<"ms: Process '"<<Processes[*itr-1].getID()<<"' unable to be added; lack of memory;";
                std::cout<<std::endl;
            }
            // if defragmentation was not required
            else if(!fit->defragReq() && defragmented == false) {
                std::cout<<"time "<<time<<"ms: "<<"Process '"<<Processes[*itr-1].getID()<<"' added to the system ";
                std::cout<<"[Q";
                queue.push_back(*itr);
                if(alg == "srt")
                   queue.sort(lower_burst_time);
                for(std::list<int>::iterator it = queue.begin(); it != queue.end(); it++)
                    std::cout<<" "<<Processes[*it-1].getID();
                std::cout<<"]";
                std::cout<<std::endl;
                std::cout<<"time "<<time<<"ms: Simulated Memory:";
                std::cout<<std::endl;
                fit->print();
            }
        }
    }
    // return amount of time taken for defragmentation
    return time_change;
}
    

// RR algorithm
void Simulation::runRR(Process* Processes, const std::string& fit_algo, int X_, int Y_) {
    
    // algorithm determination
    if(fit_algo != "Next-Fit" && fit_algo != "Best-Fit")
        fit = new FirstFit(X_,Y_);
    else {
        if(fit_algo == "Next-Fit")
            fit = new NextFit(X_,Y_);
        else if(fit_algo == "Best-Fit")
            fit = new BestFit(X_,Y_);
    }
    // running time (ms)
    long int time = 0;
    // process queue
    std::list<int> queue;
    // print msg
    std::cout<<"time "<<time<<"ms: Simulator started for RR (t_slice "<<t_slice<<") and "<<fit_algo;
    std::cout<<std::endl;
    // check for and perform defragmentation if required
    int allo = allocate(Processes, time, queue, "rr");
    // update time spent on defragmentation
    defrag_time += allo;
    // time taken to change context
    time += t_cs;
    bool context_change = false;
    long int change_strt_time = 0;
    int io_flag;
    int cpu_flag;
    
    while(true) {
        
        // check for an perform defragmentation
        int allo = allocate(Processes, time, queue, "rr");
        // update defragmentation time
        defrag_time += allo;
        // check for I/O during defragmentation
        if(allo > 0) {
            long int t = time - allo;
            while(t < time) {
                for(int j = 0; j < n; j++) {
                    if(Processes[j].inIO()) {
                        if(t - Processes[j].getIOStrtTime() == Processes[j].getIOTime()) {
                            Processes[j].resetIOStrtTime();
                            queue.push_back(Processes[j].getProcNumber());
                        }
                    }
                }
                t++;
            }
        }
        // check for processes waiting in the queue
        for(int i = 0; i < n; i++) {
            if(!Processes[i].inCPU() && !Processes[i].inIO())
                // update wait time
                Processes[i].increaseWaitTime(allo);
        }
        // delay the context switches
        if(allo > 0 && context_change == true)
            change_strt_time += allo;
        // check whether there are processes in I/O or CPU burst
        io_flag = 0; // number of processes in I/O
        cpu_flag = 0; //number of processes using CPU
        for(int j = 0; j < n; j++) {
            if(Processes[j].inIO())
                io_flag++;
            if(Processes[j].inCPU())
                cpu_flag++;
        }
        // if NO exit, else continue time increments
        if((io_flag + cpu_flag) == 0 && queue.size() == 0) {
            std::cout<<"time "<<(time-1)<<"ms: Simulator for RR and "<<fit_algo<<" ended [Q]";
            std::cout<<std::endl;
            break;
        }
        // no active proceses, non-empty queue - select new process
        else if((io_flag + cpu_flag) == 0 && queue.size() > 0
                && time > t_cs) {
            context_change = true;
            if(change_strt_time == 0)
                change_strt_time = time - 1;
        }
        // no active CPU process, but processes uperforming I/O
        else if((io_flag > 0 && cpu_flag == 0) && queue.size() > 0
                && time > t_cs) {
            context_change = true;
            if(change_strt_time == 0)
                change_strt_time = time - 1;
        }
        // if a change of context is in the process
        if(context_change == true || time == t_cs) {
            if(time - change_strt_time == t_cs || time == t_cs) {
                // update number of context changes
                num_rr_contexts++;
                // get the process that is first in the queue
                int p = queue.front();
                // remove it from the queue
                queue.pop_front();
                // print msg
                std::cout<<"time "<<time<<"ms: Process '"<<Processes[p-1].getID()<<"' started using the CPU [Q";
                for(std::list<int>::iterator it = queue.begin(); it !=
                    queue.end(); it++) {
                    std::cout<<" "<<Processes[*it-1].getID();
                }
                std::cout<<"]";
                std::cout<<std::endl;
                // set the process into CPU usage mode
                Processes[p-1].setCPUStrtTime(time);
                // update total wait time for this process
                Processes[p-1].updateTotalWaitTime(Processes[p-1].currentWaitTime() - t_cs);
                // update total turn around time
                Processes[p-1].updateTotalTurnTime(Processes[p-1].currentWaitTime());
                // reset temporary wait time
                Processes[p-1].resetWaitTime(0);
                // reset context change
                context_change = false;
                change_strt_time = 0;
            }
        }
        // check for completion of CPU bursts
        for(int i = 0; i < n; i++) {
            // if a process had been preempted due to a time slice
            if(Processes[i].inCPU() && Processes[i].sliced()) {
                Processes[i].incrementCPUBurstTime();
                Processes[i].rslice();
            }
            // if a process is in CPU burst mode and was not preempted
            if(Processes[i].inCPU() && Processes[i].sliced() == false) {
                if(Processes[i].getCPUTimeElapsed()%t_slice == 0 &&
                   Processes[i].getCPUTimeElapsed() < Processes[i].getCPUTime() && Processes[i].getCPUTimeElapsed() > 0) {
                    if(queue.size() > 0) {
                        // preempt
                        Processes[i].tslice();
                        context_change = true;
                        change_strt_time = time+1;
                        queue.push_back(Processes[i].getProcNumber());
                        std::cout<<"time "<<(time+1)<<"ms: Process '"<<Processes[i].getID()<<"' preempted due to time slice expiration [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<Processes[*it-1].getID();
                        }
                        std::cout<<"]";
                        std::cout<<std::endl;
                        continue;
                    }
                }
                // if the burst is complete
                if(Processes[i].getCPUTimeElapsed() >=
                   Processes[i].getCPUTime()) {
                    // update number of bursts
                    num_rr_bursts++;
                    // reset process to default CPU burst mode
                    Processes[i].resetCPUTimeElapsed();
                    // update the turn around time
                    Processes[i].updateTotalTurnTime(Processes[i].getCPUTime());
                    // update the burst time
                    Processes[i].updateTotalBurstTime(Processes[i].getCPUTime());
                    // reset process to default CPU burst mode
                    Processes[i].resetCPUStrtTime();
                    // update number of bursts completed
                    Processes[i].update_num_bursts();
                    // if the process has completed all its bursts
                    if(Processes[i].num_bursts() == 0) {
                        // deallocate memory
                        char ID = Processes[i].getID()[0];
                        fit->deallocate(ID);
                        // process is terminated
                        std::cout<<"time "<<time<<"ms: Process '"<<Processes[i].getID()<<"' terminated [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<Processes[*it-1].getID();
                        }
                        std::cout<<"]";
                        std::cout<<std::endl;
                        // change context if reuqired
                        if(context_change == false && change_strt_time == 0) {
                            context_change = true;
                            change_strt_time = time;
                        }
                    }
                    // if the process has not completed ALL its bursts
                    if(Processes[i].num_bursts() > 0) {
                        // print msg
                        std::cout<<"time "<<time<<"ms: Process '"<<Processes[i].getID()<<"' completed its CPU burst [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<Processes[*it-1].getID();
                        }
                        std::cout<<"]";
                        std::cout<<std::endl;
                        // if the process has I/O time
                        if(Processes[i].getIOTime() > 0) {
                            // set process I/O start time
                            Processes[i].setIOStrtTime(time);
                            // print msg
                            std::cout<<"time "<<time<<"ms: Process '"<<Processes[i].getID()<<"' performing I/O [Q";
                            for(std::list<int>::iterator it = queue.begin(); it !=
                                queue.end(); it++) {
                                std::cout<<" "<<Processes[*it-1].getID();
                            }
                            std::cout<<"]";
                            std::cout<<std::endl;
                            if(context_change == false && change_strt_time == 0) {
                                context_change = true;
                                change_strt_time = time;
                            }
                        }
                        // push the process back into the queue immediately
                        else {
                            queue.push_back(Processes[i].getProcNumber());
                            if(context_change == false && change_strt_time == 0) {
                                context_change = true;
                                change_strt_time = time;
                            }
                        }
                    }
                }
                else {
                    if(Processes[i].inCPU())
                        Processes[i].incrementCPUBurstTime();
                }
            }
        }
        
        // check for I/O completion
        for(int i = 0; i < n; i++) {
            // if a process is in I/O
            if(Processes[i].inIO()) {
                // if I/O has been completed
                if(time - Processes[i].getIOStrtTime() >= Processes[i].getIOTime()) {
                    // reset I/O start time
                    Processes[i].resetIOStrtTime();
                    // add process back to the queue
                    queue.push_back(Processes[i].getProcNumber());
                    // print msg
                    std::cout<<"time "<<time<<"ms: Process '"<<Processes[i].getID()<<"' completed I/O [Q";
                    for(std::list<int>::iterator it = queue.begin(); it !=
                        queue.end(); it++) {
                        std::cout<<" "<<Processes[*it-1].getID();
                    }
                    std::cout<<"]";
                    std::cout<<std::endl;
                }
            }
        }
        // check for processes waiting in the queue
        for(int i = 0; i < n; i++) {
            // if a process is waiting
            if(!Processes[i].inCPU() && !Processes[i].inIO())
                // update its wait time
                Processes[i].incrementWaitTime();
        }
        // increment time
        time++;
        // if the queue is empty
        if(queue.size() == 0 && context_change == true) {
            context_change = false;
            change_strt_time = 0;
        }
    }
    // compute averages
    for(int i = 0; i < n; i++) {
        avg_wait_time_rr += Processes[i].getWaitTime();
        avg_burst_time_rr += Processes[i].getBurstTime();
        avg_trn_arnd_time_rr += Processes[i].getTurnTime();
    }
    // if the number of executed bursts is zero
    if(num_rr_bursts == 0) {
        std::cerr<<"ERROR: zero executed bursts!";
        std::cout<<std::endl;
        exit(1);
    }
    // compute averages
    avg_wait_time_rr /= num_rr_bursts;
    avg_burst_time_rr /= num_rr_bursts;
    avg_trn_arnd_time_rr /= num_rr_bursts;
    defrag_time_frac = defrag_time/(double)time;  // fraction of total time spent on defragmentation
    std::cout<<std::endl;
    
    delete fit;
    fit = NULL;
    
    return;
    
}



// SRT algorithm
void Simulation::runSRT(Process* Processes, const std::string& fit_algo, int X_, int Y_) {

    SRT_processes = Processes;
    // determine algorithm
    if(fit_algo != "Next-Fit" && fit_algo != "Best-Fit")
        fit = new FirstFit(X_,Y_);
    else {
        if(fit_algo == "Next-Fit")
            fit = new NextFit(X_,Y_);
        else if(fit_algo == "Best-Fit")
            fit = new BestFit(X_,Y_);
    }
    // process queue
    std::list<int> queue;
    // running time (ms)
    long int time = 0;
    // print msg
    std::cout<<"time "<<time<<"ms: Simulator started for SRT and "<<fit_algo;
    std::cout<<std::endl;
    // check for and perform defragmentation if required
    int allo = allocate(Processes, time, queue, "srt");
    // update amount of defragmentation time spent
    defrag_time += allo;
    // time taken to change context
    time += t_cs;
    bool context_change = false;
    bool process_preempted = false;
    long int change_strt_time = 0;
    int io_flag;
    int cpu_flag;
    int current = 0;
    
    while(true) {
        
        // check for and perform defragmentation
        int allo = allocate(Processes, time, queue, "srt");
        // update time spent on defragmentation
        defrag_time += allo;
        bool context_altered = false;
        // check for I/O during defragmentation
        if(allo > 0) {
            long int t = time - allo;
            while(t < time) {
                for(int j = 0; j < n; j++) {
                  if(Processes[j].inIO()) {
                    if(t - Processes[j].getIOStrtTime() == Processes[j].getIOTime()) {
                        Processes[j].resetIOStrtTime();
                        int cur = 0;
                        for(int k = 0; k < n; k++)
                            if(Processes[k].inCPU()) {
                                cur = k+1;
                                break;
                            }
                        // preempt if required
                        if(cur > 0 && Processes[cur-1].getCPUTime() - Processes[cur-1].getCPUTimeElapsed() >
                           Processes[j].getCPUTime()) {
                            process_preempted = true;
                            Processes[cur-1].preempt();
                            Processes[cur-1].decrementCPUBurstTime();
                            queue.push_back(cur);
                            queue.sort(lower_burst_time);
                            context_change = true;
                            change_strt_time = time;
                            current = j+1;
                            context_altered = true;
                        }
                        else {
                            queue.push_back(Processes[j].getProcNumber());
                            queue.sort(lower_burst_time);
                        }
                    }
                 }
              }
             t++;
            }
        }
        
        // check for processes waiting in the queue
        for(int i = 0; i < n; i++) {
            if(!Processes[i].inCPU() && !Processes[i].inIO())
                // update wait time
                Processes[i].increaseWaitTime(allo);
        }
        if(allo > 0 && context_change == true && context_altered == false)
            change_strt_time += allo;
        // check whether there are processes in I/O or CPU burst
        io_flag = 0; // number of processes in I/O
        cpu_flag = 0; //number of processes using CPU
        for(int j = 0; j < n; j++) {
            if(Processes[j].inIO())
                io_flag++;
            if(Processes[j].inCPU())
                cpu_flag++;
        }
        // if NO exit, else continue time increments
        if((io_flag + cpu_flag) == 0 && queue.size() == 0) {
            std::cout<<"time "<<(time-1)<<"ms: Simulator for SRT and "<<fit_algo<<" ended [Q]";
            std::cout<<std::endl;
            break;
        }
        // no active proceses, non-empty queue - select new process
        else if((io_flag + cpu_flag) == 0 && queue.size() > 0
                && time > t_cs) {
            context_change = true;
            if(change_strt_time == 0)
                change_strt_time = time - 1;
        }
        // no active CPU process, but processes uperforming I/O
        else if((io_flag > 0 && cpu_flag == 0) && queue.size() > 0
                && time > t_cs) {
            context_change = true;
            if(change_strt_time == 0)
                change_strt_time = time - 1;
        }
        // if a change of context is in the process
        if(context_change == true || time == t_cs) {
            if(time - change_strt_time == t_cs || time == t_cs) {
                // update number of context changes
                num_srt_contexts++;
                // get the process that is first in the queue
                int p = queue.front();
                // if a process was preempted
                if(process_preempted == true)
                    p = current;
                else
                    // remove it from the queue
                    queue.pop_front();
                // print msg
                std::cout<<"time "<<time<<"ms: Process '"<<Processes[p-1].getID()<<"' started using the CPU [Q";
                for(std::list<int>::iterator it = queue.begin(); it !=
                    queue.end(); it++) {
                    std::cout<<" "<<Processes[*it-1].getID();
                }
                std::cout<<"]";
                std::cout<<std::endl;
                // set the process into CPU usage mode
                Processes[p-1].setCPUStrtTime(time);
                // update wait time
                Processes[p-1].updateTotalWaitTime(Processes[p-1].currentWaitTime() - t_cs);
                // update turn around time
                Processes[p-1].updateTotalTurnTime(Processes[p-1].currentWaitTime());
                // reset wait time
                Processes[p-1].resetWaitTime(0);
                if(process_preempted == false)
                    current = p;
                else
                    process_preempted = false;
                context_change = false;
                change_strt_time = 0;
            }
        }
        
        // check for completion of CPU bursts
        for(int i = 0; i < n; i++) {
            // if a process is in CPU burst mode
            if(Processes[i].inCPU()) {
                // if the burst is complete
                if(Processes[i].getCPUTimeElapsed() ==
                   Processes[i].getCPUTime()) {
                    // update number of executed bursts
                    num_srt_bursts++;
                    // update turn around time
                    Processes[i].updateTotalTurnTime(Processes[i].getCPUTimeElapsed());
                    // update burst time
                    Processes[i].updateTotalBurstTime(Processes[i].getCPUTimeElapsed());
                    // reset process to default CPU burst mode
                    Processes[i].resetCPUTimeElapsed();
                    // update number of bursts completed
                    Processes[i].update_num_bursts();
                    // if the process has completed all its bursts
                    if(Processes[i].num_bursts() == 0) {
                        char ID = Processes[i].getID()[0];
                        fit->deallocate(ID);
                        // process is terminated
                        std::cout<<"time "<<time<<"ms: Process '"<<Processes[i].getID()<<"' terminated [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<Processes[*it-1].getID();
                        }
                        std::cout<<"]";
                        std::cout<<std::endl;
                        if(context_change == false && change_strt_time == 0) {
                            context_change = true;
                            change_strt_time = time;
                            current = 0;
                        }
                    }
                    // if the process has not completed ALL its bursts
                    if(Processes[i].num_bursts() > 0) {
                        // print msg
                        std::cout<<"time "<<time<<"ms: Process '"<<Processes[i].getID()<<"' completed its CPU burst [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<Processes[*it-1].getID();
                        }
                        std::cout<<"]";
                        std::cout<<std::endl;
                        // if the process has I/O time
                        if(Processes[i].getIOTime() > 0) {
                            // set process I/O start time
                            Processes[i].setIOStrtTime(time);
                            // print msg
                            std::cout<<"time "<<time<<"ms: Process '"<<Processes[i].getID()<<"' performing I/O [Q";
                            for(std::list<int>::iterator it = queue.begin(); it !=
                                queue.end(); it++) {
                                std::cout<<" "<<Processes[*it-1].getID();
                            }
                            std::cout<<"]";
                            std::cout<<std::endl;
                            if(context_change == false && change_strt_time == 0) {
                                context_change = true;
                                change_strt_time = time;
                            }
                        }
                        // push the process back into the queue immediately
                        else {
                            // if the current process has higher burst time than the process to be added back
                            if(current > 0 && Processes[current-1].getCPUTime() - Processes[current-1].getCPUTimeElapsed() > 
                              Processes[i].getCPUTime()) {
                                // preemption occurs
                                process_preempted = true;
                                // preempt the process
                                Processes[current-1].preempt();
                                // update the burst time
                                Processes[current-1].decrementCPUBurstTime();
                                // add the current process back
                                queue.push_back(current);
                                // sort the queue
                                queue.sort(lower_burst_time);
                                std::cout<<"time "<<time<<"ms: Process '"<<Processes[current-1].getID()<<
                                  "' preempted by Process '"<<Processes[i].getID()<<"' [Q";
                                for(std::list<int>::iterator it = queue.begin(); it !=
                                    queue.end(); it++) {
                                    std::cout<<" "<<Processes[*it-1].getID();
                                }
                                std::cout<<"]";
                                std::cout<<std::endl;
                                // change current process
                                current = i + 1;
                                // context change will take place
                                context_change = true;
                                change_strt_time = time;
                            }
                            // otherwise simply continue
                            else {
                                // add the process back
                                queue.push_back(Processes[i].getProcNumber());
                                // sort the queue
                                queue.sort(lower_burst_time);
                                
                            }
                            // if a context change has to take place
                            if(context_change == false && change_strt_time == 0) {
                                context_change = true;
                                change_strt_time = time;
                            }
                        }
                    }
                }
                // update burst time
                else {
                    if(Processes[i].inCPU())
                        Processes[i].incrementCPUBurstTime();
                }
            }
        }
        
        // check for I/O completion
        for(int i = 0; i < n; i++) {
            // if a process is in I/O
            if(Processes[i].inIO()) {
                // if I/O has been completed
                if(time - Processes[i].getIOStrtTime() >= Processes[i].getIOTime()) {
                    // reset I/O start time
                    Processes[i].resetIOStrtTime();
                    // if the process has lower burst time than the remaining burst time
                    if(current > 0 && Processes[current-1].getCPUTime() - Processes[current-1].getCPUTimeElapsed() > 
                       Processes[i].getCPUTime()) {
                        // print msg
                        std::cout<<"time "<<time<<"ms: Process '"<<Processes[i].getID()<<"' completed I/O [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<Processes[*it-1].getID();
                        }
                        std::cout<<"]";
                        std::cout<<std::endl;
                        // preemption occurs
                        process_preempted = true;
                        // preempt the curent process
                        Processes[current-1].preempt();
                        // update the burst time
                        Processes[current-1].decrementCPUBurstTime();
                        // add the current process back to the queue
                        queue.push_back(current);
                        // sort the queue
                        queue.sort(lower_burst_time);
                        // print msg
                        std::cout<<"time "<<time<<"ms: Process '"<<Processes[current-1].getID()<<"' preempted by Process '"<<Processes[i].getID()<<"' [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<Processes[*it-1].getID();
                        }
                        std::cout<<"]";
                        std::cout<<std::endl;
                        // update current process
                        current = i + 1;
                        // context change will take place
                        context_change = true;
                        change_strt_time = time;
                    }
                    // otherwise continue
                    else {
                        // add the process back
                        queue.push_back(Processes[i].getProcNumber());
                        // sort the queue
                        queue.sort(lower_burst_time);
                        // print msg
                        std::cout<<"time "<<time<<"ms: Process '"<<Processes[i].getID()<<"' completed I/O [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<Processes[*it-1].getID();
                        }
                        std::cout<<"]";
                        std::cout<<std::endl;
                    }
                }    
            }
        }
        // check for processes waiting in the queue
        for(int i = 0; i < n; i++) {
            if(!Processes[i].inCPU() && !Processes[i].inIO())
                // update wait time
                Processes[i].incrementWaitTime();
        }
        // increment time
        time++;
        // if the queue is empty 
        if(queue.size() == 0 && context_change == true) {
            context_change = false;
            change_strt_time = 0;
        }
    }
    // compute averages
    for(int i = 0; i < n; i++) {
        avg_wait_time_srt += Processes[i].getWaitTime();
        avg_burst_time_srt += Processes[i].getBurstTime();
        avg_trn_arnd_time_srt += Processes[i].getTurnTime();
    }
    // if the number of executed bursts is zero
    if(num_srt_bursts == 0) {
      std::cerr<<"ERROR: zero executed bursts!";
      std::cout<<std::endl;
      exit(1);
    }
    // compute averages
    avg_wait_time_srt /= num_srt_bursts;
    avg_burst_time_srt /= num_srt_bursts;
    avg_trn_arnd_time_srt /= num_srt_bursts;
    defrag_time_frac = defrag_time/(double)time;
    std::cout<<std::endl;

    delete fit;
    fit = NULL;
    
    return;

}


 
