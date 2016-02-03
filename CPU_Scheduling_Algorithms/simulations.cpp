#include <algorithm>
#include "process_simulation.h"


Process* SRT_processes;
Process* PWA_processes;

// compare burst times
bool lower_burst_time(int x, int y) {
  return SRT_processes[x-1].getCPUTime() < SRT_processes[y-1].getCPUTime();
}
// compare priority numbers
bool higher_priority(int x, int y) {
  // compare pid for same priority
  if(PWA_processes[x-1].getPriority() == PWA_processes[y-1].getPriority())
    return x < y;
  else
    return PWA_processes[x-1].getPriority() < PWA_processes[y-1].getPriority();
}

// FCFS algorithm
void Simulation::runFCFS(std::list<int>& queue, Process* Processes) {
    
    // running time (ms)
    long int time = 0;
    // print msg
    std::cout<<"time "<<time<<"ms: Simulator started for FCFS [Q";
    for(std::list<int>::iterator it = queue.begin(); it !=
        queue.end(); it++) {
        std::cout<<" "<<*it;
    }
    std::cout<<"]";
    std::cout<<std::endl;
    // time taken to change context
    time += t_cs;
    bool context_change = false;
    long int change_strt_time = 0;
    int io_flag;
    int cpu_flag;
    
    while(true) {
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
            std::cout<<"time "<<(time-1)<<"ms: Simulator for FCFS ended [Q]";
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
                num_fcfs_contexts++;
                // get the process that is first in the queue
                int p = queue.front();
                // remove it from the queue
                queue.pop_front();
                // print msg
                std::cout<<"time "<<time<<"ms: P"<<p<<" started using the CPU [Q";
                for(std::list<int>::iterator it = queue.begin(); it !=
                    queue.end(); it++) {
                    std::cout<<" "<<*it;
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
            // if a process is n CPU burst mode
            if(Processes[i].inCPU()) {
                // if the burst is complete
                if(time - Processes[i].getCPUStrtTime() ==
                   Processes[i].getCPUTime()) {
                    // update number of bursts
                    num_fcfs_bursts++;
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
                        // process is terminated
                        std::cout<<"time "<<time<<"ms: P"<<Processes[i].getProcNumber()<<" terminated [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<*it;
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
                        std::cout<<"time "<<time<<"ms: P"<<Processes[i].getProcNumber()<<" completed its CPU burst [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<*it;
                        }
                        std::cout<<"]";
                        std::cout<<std::endl;
                        // if the process has I/O time
                        if(Processes[i].getIOTime() > 0) {
                            // set process I/O start time
                            Processes[i].setIOStrtTime(time);
                            // print msg
                            std::cout<<"time "<<time<<"ms: P"<<Processes[i].getProcNumber()<<" performing I/O [Q";
                            for(std::list<int>::iterator it = queue.begin(); it !=
                                queue.end(); it++) {
                                std::cout<<" "<<*it;
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
            }
        }
        
        // check for I/O completion
        for(int i = 0; i < n; i++) {
            // if a process is in I/O
            if(Processes[i].inIO()) {
                // if I/O has been completed
                if(time - Processes[i].getIOStrtTime() == Processes[i].getIOTime()) {
                    // reset I/O start time
                    Processes[i].resetIOStrtTime();
                    // add process back to the queue
                    queue.push_back(Processes[i].getProcNumber());
                    // print msg
		    std::cout<<"time "<<time<<"ms: P"<<Processes[i].getProcNumber()<<" completed I/O [Q";
                    for(std::list<int>::iterator it = queue.begin(); it !=
                        queue.end(); it++) {
                        std::cout<<" "<<*it;
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
        avg_wait_time_fcfs += Processes[i].getWaitTime();
        avg_burst_time_fcfs += Processes[i].getBurstTime();
        avg_trn_arnd_time_fcfs += Processes[i].getTurnTime();
    }
    // if the number of executed bursts is zero
    if(num_fcfs_bursts == 0) {
      std::cerr<<"ERROR: zero executed bursts!";
      std::cout<<std::endl;
      exit(1);
    }
    // compute averages
    avg_wait_time_fcfs /= num_fcfs_bursts;
    avg_burst_time_fcfs /= num_fcfs_bursts;
    avg_trn_arnd_time_fcfs /= num_fcfs_bursts;    
    std::cout<<std::endl;    

    return;

}

// SRT algorithm
void Simulation::runSRT(std::list<int>& queue, Process* Processes) {

    SRT_processes = Processes;
    // sort by burst time
    queue.sort(lower_burst_time);
    // running time (ms)
    long int time = 0;
    // print msg
    std::cout<<"time "<<time<<"ms: Simulator started for SRT [Q";
    for(std::list<int>::iterator it = queue.begin(); it !=
        queue.end(); it++) {
        std::cout<<" "<<*it;
    }
    std::cout<<"]";
    std::cout<<std::endl;
    // time taken to change context
    time += t_cs;
    bool context_change = false;
    bool process_preempted = false;
    long int change_strt_time = 0;
    int io_flag;
    int cpu_flag;
    int current = 0;
    
    while(true) {
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
            std::cout<<"time "<<(time-1)<<"ms: Simulator for SRT ended [Q]";
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
                std::cout<<"time "<<time<<"ms: P"<<p<<" started using the CPU [Q";
                for(std::list<int>::iterator it = queue.begin(); it !=
                    queue.end(); it++) {
                    std::cout<<" "<<*it;
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
                        // process is terminated
                        std::cout<<"time "<<time<<"ms: P"<<Processes[i].getProcNumber()<<" terminated [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<*it;
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
                        std::cout<<"time "<<time<<"ms: P"<<Processes[i].getProcNumber()<<" completed its CPU burst [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<*it;
                        }
                        std::cout<<"]";
                        std::cout<<std::endl;
                        // if the process has I/O time
                        if(Processes[i].getIOTime() > 0) {
                            // set process I/O start time
                            Processes[i].setIOStrtTime(time);
                            // print msg
                            std::cout<<"time "<<time<<"ms: P"<<Processes[i].getProcNumber()<<" performing I/O [Q";
                            for(std::list<int>::iterator it = queue.begin(); it !=
                                queue.end(); it++) {
                                std::cout<<" "<<*it;
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
                                std::cout<<"time "<<time<<"ms: P"<<current<<" preempted by P"<<i+1<<" [Q";
                                for(std::list<int>::iterator it = queue.begin(); it !=
                                    queue.end(); it++) {
                                    std::cout<<" "<<*it;
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
                if(time - Processes[i].getIOStrtTime() == Processes[i].getIOTime()) {
                    // reset I/O start time
                    Processes[i].resetIOStrtTime();
                    // if the process has lower burst time than the remaining burst time
                    if(current > 0 && Processes[current-1].getCPUTime() - Processes[current-1].getCPUTimeElapsed() > 
                       Processes[i].getCPUTime()) {
                        // print msg
                        std::cout<<"time "<<time<<"ms: P"<<Processes[i].getProcNumber()<<" completed I/O [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<*it;
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
                        std::cout<<"time "<<time<<"ms: P"<<current<<" preempted by P"<<(i+1)<<" [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<*it;
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
                        std::cout<<"time "<<time<<"ms: P"<<Processes[i].getProcNumber()<<" completed I/O [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<*it;
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
    std::cout<<std::endl;

    return;

}

// PWA algorithm
void Simulation::runPWA(std::list<int>& queue, Process* Processes) {

    PWA_processes = Processes;
    // sort queue by priority
    queue.sort(higher_priority);
    // running time (ms)
    long int time = 0;
    // print msg
    std::cout<<"time "<<time<<"ms: Simulator started for PWA [Q";
    for(std::list<int>::iterator it = queue.begin(); it !=
	  queue.end(); it++) {
      std::cout<<" "<<*it;
    }
    std::cout<<"]";
    std::cout<<std::endl;
    // first context change
    time += t_cs;
  
    bool context_change = false;
    bool process_preempted = false;
    long int change_strt_time = 0;
    int io_flag;
    int cpu_flag;
    // current running process
    int current = 0;
    
    while(true) {
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
            std::cout<<"time "<<(time-1)<<"ms: Simulator for PWA ended [Q]";
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
        // no active CPU process, but processes performing I/O
        else if((io_flag > 0 && cpu_flag == 0) && queue.size() > 0
                && time > t_cs) {
            context_change = true;
            if(change_strt_time == 0)
                change_strt_time = time - 1;
        }

        
        // if a change of context is in the process
        if(context_change == true || time == t_cs) {
            if(time - change_strt_time == t_cs || time == t_cs) {
                // update the number of context changes
                num_pwa_contexts++;
                // get the process that is first in the queue
                int p = queue.front();
                // if a process was preempted
                if(process_preempted == true)
                    p = current;
                else
                    // remove it from the queue
                    queue.pop_front();
                // print msg
                std::cout<<"time "<<time<<"ms: P"<<p<<" started using the CPU [Q";
                for(std::list<int>::iterator it = queue.begin(); it !=
                    queue.end(); it++) {
                    std::cout<<" "<<*it;
                }
                std::cout<<"]";
                std::cout<<std::endl;
                // set the process into CPU usage mode
                Processes[p-1].setCPUStrtTime(time);
                // update total wait time
                Processes[p-1].updateTotalWaitTime(Processes[p-1].currentWaitTime() - t_cs);
                // update total turn around time
                Processes[p-1].updateTotalTurnTime(Processes[p-1].currentWaitTime());
                // reset wait time for the process
                Processes[p-1].resetWaitTime(0);
                if(process_preempted == false)
                    current = p;
                else
                    process_preempted = false;
                context_change = false;
                change_strt_time = 0;
            }
        }
        // sort by priority
        queue.sort(higher_priority);
        // check for completion of CPU bursts
        for(int i = 0; i < n; i++) {
            // if a process is in CPU burst mode
            if(Processes[i].inCPU()) {
                // if the burst is complete
                if(Processes[i].getCPUTimeElapsed() ==
                   Processes[i].getCPUTime()) {
                    // update number of executed bursts
                    num_pwa_bursts++;
                    // update the turn around time
                    Processes[i].updateTotalTurnTime(Processes[i].getCPUTimeElapsed());
                    // update the burst time
                    Processes[i].updateTotalBurstTime(Processes[i].getCPUTimeElapsed());
                    // reset process to default CPU burst mode
                    Processes[i].resetCPUTimeElapsed();
                    // update number of bursts completed
                    Processes[i].update_num_bursts();
                    // if the process has completed all its bursts
                    if(Processes[i].num_bursts() == 0) {
                        // process is terminated
                        std::cout<<"time "<<time<<"ms: P"<<Processes[i].getProcNumber()<<" terminated [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<*it;
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
                        std::cout<<"time "<<time<<"ms: P"<<Processes[i].getProcNumber()<<" completed its CPU burst [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<*it;
                        }
                        std::cout<<"]";
                        std::cout<<std::endl;
                        // if the process has I/O time
                        if(Processes[i].getIOTime() > 0) {
                            // set process I/O start time
                            Processes[i].setIOStrtTime(time);
                            // print msg
                            std::cout<<"time "<<time<<"ms: P"<<Processes[i].getProcNumber()<<" performing I/O [Q";
                            for(std::list<int>::iterator it = queue.begin(); it !=
                                queue.end(); it++) {
                                std::cout<<" "<<*it;
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
                            // but if the process has higher priority than the current process
                            if(current > 0 && Processes[i].getPriority() < Processes[current-1].getPriority()) {
                                // preemption occurs
                                process_preempted = true;
                                // preempt the process
                                Processes[current-1].preempt();
                                // update burst time
                                Processes[current-1].decrementCPUBurstTime();
                                // push the process back
                                queue.push_back(current);
                                // sort by priority
                                queue.sort(higher_priority);
                                std::cout<<"time "<<time<<"ms: P"<<current<<" preempted by P"<<i+1<<" [Q";
                                for(std::list<int>::iterator it = queue.begin(); it !=
                                    queue.end(); it++) {
                                    std::cout<<" "<<*it;
                                }
                                std::cout<<"]";
                                std::cout<<std::endl;
                                // change the current process
                                current = i + 1;
                                // set context change to occur
                                context_change = true;
                                change_strt_time = time;
                            }
                            else {
                                // else simply add the process back
                                queue.push_back(Processes[i].getProcNumber());
                                // sort by priority
                                queue.sort(higher_priority);
                                
                            }
                            // if a context change must take place
                            if(context_change == false && change_strt_time == 0) {
                                context_change = true;
                                change_strt_time = time;
                            }
                        }
                    }
                }
                else {
                    // update burst times
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
                if(time - Processes[i].getIOStrtTime() == Processes[i].getIOTime()) {
                    // reset I/O start time
                    Processes[i].resetIOStrtTime();
                    // if the process has higher priority than current
                    if(current > 0 && Processes[i].getPriority() < Processes[current-1].getPriority()) {
                        // print msg
                        std::cout<<"time "<<time<<"ms: P"<<Processes[i].getProcNumber()<<" completed I/O [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<*it;
                        }
                        std::cout<<"]";
                        std::cout<<std::endl;
                        // preemption occurs
                        process_preempted = true;
                        // preempt process
                        Processes[current-1].preempt();
                        // update burst time
                        Processes[current-1].decrementCPUBurstTime();
                        // add current process to the queue
                        queue.push_back(current);
                        // sort by priority
                        queue.sort(higher_priority);
                        // print msg
                        std::cout<<"time "<<time<<"ms: P"<<current<<" preempted by P"<<(i+1)<<" [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<*it;
                        }
                        std::cout<<"]";
                        std::cout<<std::endl;
                        // change current process
                        current = i + 1;
                        context_change = true;
                        change_strt_time = time;
                    }
                    else {
                        // othewise add the process back to the queue
                        queue.push_back(Processes[i].getProcNumber());
                        // sort by priority
                        queue.sort(higher_priority);
                        // print msg
                        std::cout<<"time "<<time<<"ms: P"<<Processes[i].getProcNumber()<<" completed I/O [Q";
                        for(std::list<int>::iterator it = queue.begin(); it !=
                            queue.end(); it++) {
                            std::cout<<" "<<*it;
                        }
                        std::cout<<"]";
                        std::cout<<std::endl;
                    }
                }
            }
        }
        
        // if a process at the front of the queue has higher priority
        if(queue.size() > 0 && process_preempted == false && context_change == false &&
          Processes[queue.front()-1].getPriority() < Processes[current-1].getPriority()) {
            // preemption occurs
            process_preempted = true;
            // preempt the process
            Processes[current-1].preempt();
            // update burst time
            Processes[current-1].decrementCPUBurstTime();
            // get the process at the front
            int p = queue.front();
            // remove this process
            queue.pop_front();
            // add back the currrent process
            queue.push_back(current);
            // sort by priority
            queue.sort(higher_priority);
            // print msg
            std::cout<<"time "<<time<<"ms: P"<<current<<" preempted by P"<<p<<" [Q";
            for(std::list<int>::iterator it = queue.begin(); it !=
                queue.end(); it++) {
                std::cout<<" "<<*it;
            }
            std::cout<<"]";
            std::cout<<std::endl;
            // change current process
            current = p;
            context_change = true;
            change_strt_time = time;
        }
        // increment time
        time++;
        // check for processes waiting in the queue
        for(int i = 0; i < n; i++) {
            if(!Processes[i].inCPU() && !Processes[i].inIO())
                Processes[i].incrementWaitTime();
            // if the wait time of a process exceeds the threshold
            if(Processes[i].currentWaitTime() > 3*Processes[i].getCPUTime() && 
              Processes[i].priorityUpdated() == false)
                 Processes[i].increasePriority();
        }
        // if the queue is empty 
        if(queue.size() == 0 && context_change == true) {
            context_change = false;
            change_strt_time = 0;
        }
    }
    // compute averages
    for(int i = 0; i < n; i++) {
        avg_wait_time_pwa += Processes[i].getWaitTime();
        avg_burst_time_pwa += Processes[i].getBurstTime();
        avg_trn_arnd_time_pwa += Processes[i].getTurnTime();
    }
    // if no bursts are executed
    if(num_pwa_bursts == 0) {
      std::cerr<<"ERROR: zero bursts executed!";
      std::cout<<std::endl;
      exit(1);
    }
    // compute averages
    avg_wait_time_pwa /= num_pwa_bursts;
    avg_burst_time_pwa /= num_pwa_bursts;
    avg_trn_arnd_time_pwa /= num_pwa_bursts;
    std::cout<<std::endl;

    return;

}

 
