#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <string>
#include <cstdlib>
#include <algorithm>
#include "process_simulation.h"

Process* fcfs_processes;
Process* pwa_processes;
Process* srt_processes;

// split line from input file
std::vector<std::string> split(const std::string& s , char delim) {
  // vector to store the temporal data
  std::vector<std::string> split;
  std::string temp = "";
  // split string on '|'
  for(unsigned int i = 0; i < s.size(); i++) {
    if(s[i] == delim) {
      std::string num(temp);
      split.push_back(num);
      temp = "";
    }
    else
      temp += s[i];
  }
  split.push_back(temp);
  return split;

}


int main(int argc, char* argv[]) {
  // arguments: file.txt num_processes t_cs
  if(argc != 4) {
    std::cerr<<"error: incorrect number of input arguments";
    std::cout<<std::endl;
    exit(1);
  }
  // number of processes
  int n = atoi(argv[2]);
  // context change time
  int t_cs = atoi(argv[3]);
  if(n <= 0 || t_cs <= 0) {
    std::cerr<<"error: n > 0, t_cs > 0, default t_cs = 13";
    std::cout<<std::endl;
    exit(1);
  }
  // load input file
  std::ifstream file(argv[1]);
  if(!file) {
    std::cerr<<"error: cannot read/find input file";
    std::cout<<std::endl;
    exit(1);
  }
  // array of processes
  fcfs_processes = new Process[n];
  srt_processes = new Process[n];
  pwa_processes = new Process[n];
  // queue of processes
  std::list<int> fcfs_queue;
  std::list<int> pwa_queue;
  std::list<int> srt_queue;
  // read the input file
  std::string line;
  while(getline(file , line)) {
    // if the line is not a comment and not blank
    if(line[0] != '#' && line != "") {
      // get data for each process
      std::vector<std::string> nums;
      std::string seg;
      nums = split(line , '|'); // numbers partitioned by '|'
      int proc_number = atoi(nums[0].c_str());
      int cpu_time = atoi(nums[1].c_str());
      int num_bursts = atoi(nums[2].c_str());
      int io_time = atoi(nums[3].c_str());
      int priority = atoi(nums[4].c_str());
      // new process object
      Process P(proc_number, cpu_time, num_bursts, io_time, priority);
      // populate the queues
      fcfs_processes[proc_number - 1] = P;
      fcfs_processes[proc_number - 1].resetWaitTime(t_cs);
      fcfs_queue.push_back(P.getProcNumber());
      srt_processes[proc_number - 1] = P;
      srt_processes[proc_number - 1].resetWaitTime(t_cs);
      srt_queue.push_back(P.getProcNumber());
      pwa_processes[proc_number - 1] = P;
      pwa_processes[proc_number - 1].resetWaitTime(t_cs);
      pwa_queue.push_back(P.getProcNumber());
    }
  }
  // close file
  file.close();
  // simulation of all three algorithms
  Simulation s(t_cs, n);
  // run FCFS algorithm
  s.runFCFS(fcfs_queue, fcfs_processes);
  // run SRT algorithm
  s.runSRT(srt_queue, srt_processes);
  // run PWA algorithm
  s.runPWA(pwa_queue, pwa_processes);  
  // free dynamically allocated memory
  delete [] fcfs_processes;
  fcfs_processes = NULL;
  delete [] srt_processes;
  srt_processes = NULL;
  delete [] pwa_processes;
  pwa_processes = NULL;

  // output file with stats
  std::ofstream output("simout.txt");
  // output results of FCFS
  output<<"Algorithm FCFS";
  output<<std::endl;
  output<<"-- average CPU burst time: "<<s.getFCFSBurstTime();
  output<<std::endl;
  output<<"-- average wait time: "<<s.getFCFSWaitTime();
  output<<std::endl;
  output<<"-- average turnaround time: "<<s.getFCFSTurnAroundTime();
  output<<std::endl;
  output<<"-- total number of context switches: "<<s.getFCFSContextSwitches();
  output<<std::endl;
  // output results for SRT
  output<<"Algorithm SRT";
  output<<std::endl;
  output<<"-- average CPU burst time: "<<s.getSRTBurstTime();
  output<<std::endl;
  output<<"-- average wait time: "<<s.getSRTWaitTime();
  output<<std::endl;
  output<<"-- average turnaround time: "<<s.getSRTTurnAroundTime();
  output<<std::endl;
  output<<"-- total number of context switches: "<<s.getSRTContextSwitches();
  output<<std::endl;
  // output results for PWA
  output<<"Algorithm PWA";
  output<<std::endl;
  output<<"-- average CPU burst time: "<<s.getPWABurstTime();
  output<<std::endl;
  output<<"-- average wait time: "<<s.getPWAWaitTime();
  output<<std::endl;
  output<<"-- average turnaround time: "<<s.getPWATurnAroundTime();
  output<<std::endl;
  output<<"-- total number of context switches: "<<s.getPWAContextSwitches();
  output<<std::endl;

  return 0;

}


    
 
