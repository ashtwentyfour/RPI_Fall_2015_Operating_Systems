#include <list>
#include "process.h"
#include "fit_algos.h"

class Simulation {

 public:
  // cosntructor
  Simulation(int t_con, int t_move, int t_sl, int num_proc) {
    // variable initialization
    n = num_proc;
    t_cs = t_con;
    t_slice = t_sl;
    fit = NULL;
    t_memmove = t_move;
    num_srt_bursts = 0;
    num_srt_contexts = 0;
    avg_burst_time_srt = 0.0;
    avg_wait_time_srt = 0.0;
    avg_trn_arnd_time_srt = 0.0;
    num_rr_bursts = 0;
    num_rr_contexts = 0;
    avg_burst_time_rr = 0.0;
    avg_wait_time_rr = 0.0;
    avg_trn_arnd_time_rr = 0.0;
    defrag_time = 0;
    defrag_time_frac = 0.0;
      
  }
 
  ~Simulation() {
      if(fit != NULL) delete fit;
      fit = NULL;
  }
  // run SRT algorithm
  void runSRT(Process* p, const std::string& fit_algo, int x_lim, int y_lim);
  // run RR algorithm
  void runRR(Process* p, const std::string& fit_algo, int x_lim, int y_lim);
  // defragmentation process
  int allocate(Process* processes, long int& time, std::list<int>& queue, std::string alg);
  // get SRT stats
  long double getSRTWaitTime() const {
    return avg_wait_time_srt;
  }
  long double getSRTBurstTime() const {
    return avg_burst_time_srt;
  }
  long double getSRTTurnAroundTime() const {
    return avg_trn_arnd_time_srt;
  }
  int getSRTContextSwitches() const {
    return num_srt_contexts;
  }
  // get RR stats
  long double getRRWaitTime() const {
    return avg_wait_time_rr;
  }
  long double getRRBurstTime() const {
    return avg_burst_time_rr;
  }
  long double getRRTurnAroundTime() const {
    return avg_trn_arnd_time_rr;
  }
  int getRRContextSwitches() const {
    return num_rr_contexts;
  }
  int getFragTime() const {
        return defrag_time;
  }
  long double getFragFrac() const {
       return defrag_time_frac;
  }
  // void set fcfs ordering
  void setFCFSQueue(std::list<std::string>& fcfs) {
     for(std::list<std::string>::iterator it = fcfs.begin(); it != fcfs.end(); it++)
            fcfs_order.push_back(*it);
  }
  // reset parameters
  void reset() {
      num_srt_bursts = 0;
      num_srt_contexts = 0;
      avg_burst_time_srt = 0.0;
      avg_wait_time_srt = 0.0;
      avg_trn_arnd_time_srt = 0.0;
      num_rr_bursts = 0;
      num_rr_contexts = 0;
      avg_burst_time_rr = 0.0;
      avg_wait_time_rr = 0.0;
      avg_trn_arnd_time_rr = 0.0;
      defrag_time = 0;
      defrag_time_frac = 0.0;
      if(fit != NULL)
          delete fit;
      fit = NULL;
  }

 
 private:

  // context change time
  int t_cs;
  // time slice
  int t_slice;
  // time to move 1 unit of memory
  int t_memmove;
  // number of processes
  int n;
  // fit algorithm
  FitAlgo* fit;
  int num_srt_bursts;
  int num_srt_contexts;
  int num_rr_bursts;
  int num_rr_contexts;
  int defrag_time;  // time spent on defragmentation
  long double avg_burst_time_srt;
  long double avg_wait_time_srt;
  long double avg_trn_arnd_time_srt;
  long double avg_burst_time_rr;
  long double avg_wait_time_rr;
  long double avg_trn_arnd_time_rr;
  long double defrag_time_frac;
  std::list<std::string> fcfs_order; // order in which the processes are read from the file
  
}; 
