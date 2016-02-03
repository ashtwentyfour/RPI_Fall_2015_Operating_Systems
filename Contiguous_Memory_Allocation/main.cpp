#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include "process_simulation.h"


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
  if(argc != 6) {
    std::cerr<<"error: incorrect number of input arguments";
    std::cout<<std::endl;
    exit(1);
  }
  // number of processes
  int n = atoi(argv[2]);
  // context change time
  int t_cs = atoi(argv[3]);
  // time to move 1 unit of memory
  int t_memmove = atoi(argv[4]);
  if(n <= 0 || t_cs <= 0 || t_memmove <= 0) {
    std::cerr<<"error: n > 0, t_cs > 0, t_memmove > 0, default t_cs = 13, default t_memmove = 10";
    std::cout<<std::endl;
    exit(1);
  }
  // time slice
  int t_slice = atoi(argv[5]);
  if(t_slice < 80) {
      std::cerr<<"error: t_slice >= 80";
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
  Process* srt_firstfit = new Process[n];
  Process* srt_nextfit = new Process[n];
  Process* srt_bestfit = new Process[n];
  Process* rr_firstfit = new Process[n];
  Process* rr_nextfit = new Process[n];
  Process* rr_bestfit = new Process[n];
  // read the input file
  std::string line;
  // process ID -> process
  std::map<std::string, Process> process_nums;
  // fcfs order of the processes
  std::list<std::string> fcfs_order;
  while(getline(file , line)) {
    // if the line is not a comment and not blank
    if(line[0] != '#' && line != "") {
      // get data for each process
      std::vector<std::string> nums;
      std::string seg;
      nums = split(line , '|'); // numbers partitioned by '|'
      // process parameters
      std::string proc_number = nums[0];
      fcfs_order.push_back(nums[0]);
      int arrive_time = atoi(nums[1].c_str());
      int cpu_time = atoi(nums[2].c_str());
      int num_bursts = atoi(nums[3].c_str());
      int io_time = atoi(nums[4].c_str());
      int memory = atoi(nums[5].c_str());
      // new process object
      Process P(proc_number, arrive_time, cpu_time, 
            num_bursts, io_time, memory);
      // form process ID -> process mapping
      process_nums[nums[0]] = P;
      process_nums[nums[0]].resetWaitTime(t_cs);
    }
  }
  // index
  int i = 0;
  for(std::map<std::string,Process>::iterator it = process_nums.begin();
      it != process_nums.end(); it++) {
      srt_firstfit[i] = it->second;
      srt_firstfit[i].setProcNumber(i+1);
      srt_nextfit[i] = it->second;
      srt_nextfit[i].setProcNumber(i+1);
      srt_bestfit[i] = it->second;
      srt_bestfit[i].setProcNumber(i+1);
      rr_firstfit[i] = it->second;
      rr_firstfit[i].setProcNumber(i+1);
      rr_nextfit[i] = it->second;
      rr_nextfit[i].setProcNumber(i+1);
      rr_bestfit[i] = it->second;
      rr_bestfit[i].setProcNumber(i+1);
      i++;
  }
  // close file
  file.close();
  // simulation of all six algorithms (combinations)
  Simulation s(t_cs, t_memmove, t_slice, n);

  // output file with stats
  std::ofstream output("simout.txt");
  // output results for SRT
  s.setFCFSQueue(fcfs_order);
  s.runSRT(srt_firstfit, "First-Fit", 8, 32);
  output<<"Algorithm SRT and First-Fit";
  output<<std::endl;
  output<<"-- average CPU burst time: "<<s.getSRTBurstTime();
  output<<std::endl;
  output<<"-- average wait time: "<<s.getSRTWaitTime();
  output<<std::endl;
  output<<"-- average turnaround time: "<<s.getSRTTurnAroundTime();
  output<<std::endl;
  output<<"-- total number of context switches: "<<s.getSRTContextSwitches();
  output<<std::endl;
  output<<"-- total defragmentation time: "<<s.getFragTime();
  output<<std::endl;
  output<<"-- fraction (%) of time spent performing defragmentation: "<<s.getFragFrac()*100;
  output<<std::endl;
  s.reset();
  
  s.runSRT(srt_nextfit, "Next-Fit", 8, 32);
  output<<"Algorithm SRT and Next-Fit";
  output<<std::endl;
  output<<"-- average CPU burst time: "<<s.getSRTBurstTime();
  output<<std::endl;
  output<<"-- average wait time: "<<s.getSRTWaitTime();
  output<<std::endl;
  output<<"-- average turnaround time: "<<s.getSRTTurnAroundTime();
  output<<std::endl;
  output<<"-- total number of context switches: "<<s.getSRTContextSwitches();
  output<<std::endl;
  output<<"-- total defragmentation time: "<<s.getFragTime();
  output<<std::endl;
  output<<"-- fraction (%) of time spent performing defragmentation: "<<s.getFragFrac()*100;
  output<<std::endl;
  s.reset();

  s.runSRT(srt_bestfit, "Best-Fit", 8, 32);
  output<<"Algorithm SRT and Best-Fit";
  output<<std::endl;
  output<<"-- average CPU burst time: "<<s.getSRTBurstTime();
  output<<std::endl;
  output<<"-- average wait time: "<<s.getSRTWaitTime();
  output<<std::endl;
  output<<"-- average turnaround time: "<<s.getSRTTurnAroundTime();
  output<<std::endl;
  output<<"-- total number of context switches: "<<s.getSRTContextSwitches();
  output<<std::endl;
  output<<"-- total defragmentation time: "<<s.getFragTime();
  output<<std::endl;
  output<<"-- fraction (%) of time spent performing defragmentation: "<<s.getFragFrac()*100;
  output<<std::endl;
  s.reset();
    
  s.runRR(rr_firstfit, "First-Fit", 8, 32);
  output<<"Algorithm RR and First-Fit";
  output<<std::endl;
  output<<"-- average CPU burst time: "<<s.getRRBurstTime();
  output<<std::endl;
  output<<"-- average wait time: "<<s.getRRWaitTime();
  output<<std::endl;
  output<<"-- average turnaround time: "<<s.getRRTurnAroundTime();
  output<<std::endl;
  output<<"-- total number of context switches: "<<s.getRRContextSwitches();
  output<<std::endl;
  output<<"-- total defragmentation time: "<<s.getFragTime();
  output<<std::endl;
  output<<"-- fraction (%) of time spent performing defragmentation: "<<s.getFragFrac()*100;
  output<<std::endl;
  s.reset();
    
  s.runRR(rr_nextfit, "Next-Fit", 8, 32);
  output<<"Algorithm RR and Next-Fit";
  output<<std::endl;
  output<<"-- average CPU burst time: "<<s.getRRBurstTime();
  output<<std::endl;
  output<<"-- average wait time: "<<s.getRRWaitTime();
  output<<std::endl;
  output<<"-- average turnaround time: "<<s.getRRTurnAroundTime();
  output<<std::endl;
  output<<"-- total number of context switches: "<<s.getRRContextSwitches();
  output<<std::endl;
  output<<"-- total defragmentation time: "<<s.getFragTime();
  output<<std::endl;
  output<<"-- fraction (%) of time spent performing defragmentation: "<<s.getFragFrac()*100;
  output<<std::endl;
  s.reset();

  s.runRR(rr_bestfit, "Best-Fit", 8, 32);
  output<<"Algorithm RR and Best-Fit";
  output<<std::endl;
  output<<"-- average CPU burst time: "<<s.getRRBurstTime();
  output<<std::endl;
  output<<"-- average wait time: "<<s.getRRWaitTime();
  output<<std::endl;
  output<<"-- average turnaround time: "<<s.getRRTurnAroundTime();
  output<<std::endl;
  output<<"-- total number of context switches: "<<s.getRRContextSwitches();
  output<<std::endl;
  output<<"-- total defragmentation time: "<<s.getFragTime();
  output<<std::endl;
  output<<"-- fraction (%) of time spent performing defragmentation: "<<s.getFragFrac()*100;
  output<<std::endl;
  s.reset();
    
  // free dynamically allocated memory
  delete [] srt_firstfit;
  srt_firstfit = NULL;
  delete [] srt_nextfit;
  srt_nextfit = NULL;
  delete [] srt_bestfit;
  srt_bestfit = NULL;
  delete [] rr_firstfit;
  rr_firstfit = NULL;
  delete [] rr_nextfit;
  rr_nextfit = NULL;
  delete [] rr_bestfit;
  rr_bestfit = NULL;

  return 0;

}


    
 
