#include <iostream>

/* objects of this class represent Process objects */

class Process {

 public:

  Process(const int& proc_no, const long int& b_time, 
	  const long int& n_burst, const long int& io_t, const int& p) {
   
    bursts = n_burst;
    proc_number = proc_no;
    burst_time = b_time;
    io_time = io_t;  
    in_io = false;
    in_cpu_burst = false;  
    io_start_time = 0;
    cpu_bur_strt_time = 0;
    priority = p;
    cpu_time_elapsed = 0; 
    current_wait_time = 0;
    wait_time = 0;
    priority_increased = false; 
    total_burst_time = 0;
    total_turnaround_time = 0;  
  }

  // default
  Process() {}
  
  // true if priority was increased
  bool priorityUpdated() const {
    return priority_increased;
  }
  // decrease number of bursts remaining
  void update_num_bursts() {
    bursts--;
  } 
  // get number of CPU bursts 
  int num_bursts() const {
    return bursts;
  }
  // return true if process is in I/O
  bool inIO() const {
    return in_io;
  }
  // return true if process is in CPU burst
  bool inCPU() const {
    return in_cpu_burst;
  }
  // return process number
  long int getProcNumber() const {
    return proc_number;
  }
  // get current CPU start time of the Process
  long int getCPUStrtTime() const {
    return cpu_bur_strt_time;
  }
  // get current I/O start time of the process
  long int getIOStrtTime() const {
    return io_start_time;
  }
  // reset process I/O time and status
  void resetIOStrtTime() {
    io_start_time = 0;
    in_io = false;
  }
  // reset process CPU start time and status
  void resetCPUStrtTime() {
    cpu_bur_strt_time = 0;
    in_cpu_burst = false;
  }
  // increment CPU burst time
  void incrementCPUBurstTime() {
    cpu_time_elapsed++;
  }
  // decrement CPU burst time
  void decrementCPUBurstTime() {
    cpu_time_elapsed--;
  }
  // reset CPU Time elapsed
  void resetCPUTimeElapsed() {
    cpu_time_elapsed = 0;
    in_cpu_burst = false;
  }
  // process preempted 
  void preempt() {
    in_cpu_burst = false;
  }
  // get CPU time elapsed
  long int getCPUTimeElapsed() const {
    return cpu_time_elapsed;
  }
  // set CPU start time of the process 
  void setCPUStrtTime(long int t) {
    cpu_bur_strt_time = t;
    in_cpu_burst = true;
    priority_increased = false;
  }
  // set I/O start time of the process
  void setIOStrtTime(long int t) {
    io_start_time = t;
    in_io = true;
  }
  // return I/O time for the process
  long int getIOTime() const {
    return io_time;
  }
  // return CPU burst time for the process
  long int getCPUTime() const {
    return burst_time;
  } 
  // get process priority
  int getPriority() const {
    return priority;
  }
  // get current wait time
  long int currentWaitTime() const {
    return current_wait_time;
  }
  // total wait time
  long int getWaitTime() const {
    return wait_time;
  }
  // reset wait time
  void resetWaitTime(long int x) {
    current_wait_time = x;
  }
  // increment wait time
  void incrementWaitTime() {
    current_wait_time++;
  }
  // update overall wait time
  void updateTotalWaitTime(long int x) {
    wait_time += x;
  }
  // increase priority
  void increasePriority() {
    if(priority > 0) {
      priority--;
      priority_increased = true;
    }
  }  
  // update total burst time
  void updateTotalBurstTime(long int x) {
    total_burst_time += x;
  }
  // get total burst time
  long int getBurstTime() const {
    return total_burst_time;
  }
  // update total turnaround time
  void updateTotalTurnTime(long int x) {
    total_turnaround_time += x;
  }
  // get total turnaround time
  long int getTurnTime() const {
    return total_turnaround_time;
  }
    
 
 private:

  int bursts; // number of CPU bursts
  long int io_time; // I/O process time 
  long int burst_time; // time taken for CPU burst 
  int proc_number; // process ID
  bool in_io;
  bool in_cpu_burst; 
  long int io_start_time; // start time of I/O
  long int cpu_bur_strt_time; // start time of CPU burst
  int priority; // process priority
  long int cpu_time_elapsed; // amount of cpu burst timr elapsed
  long int wait_time;
  long int current_wait_time;
  bool priority_increased;
  long int total_burst_time;
  long int total_turnaround_time;
};
