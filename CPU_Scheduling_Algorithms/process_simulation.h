#include <list>
#include "process.h"

class Simulation {

 public:
  // cosntructor
  Simulation(int t_con, int num_proc) {
    // variable initialization
    n = num_proc;
    t_cs = t_con;
    num_fcfs_bursts = 0;
    num_srt_bursts = 0;
    num_pwa_bursts = 0;
    num_fcfs_contexts = 0;
    num_srt_contexts = 0;
    num_pwa_contexts = 0;
    avg_burst_time_fcfs = 0.0;
    avg_burst_time_srt = 0.0;
    avg_burst_time_pwa = 0.0;
    avg_wait_time_fcfs = 0.0;
    avg_wait_time_srt = 0.0;
    avg_wait_time_pwa = 0.0;
    avg_trn_arnd_time_fcfs = 0.0;
    avg_trn_arnd_time_srt = 0.0;
    avg_trn_arnd_time_pwa = 0.0;
  
  }
 
  // run the FCFS algorithm
  void runFCFS(std::list<int>& queue, Process* p);
  // run the SRT algorithm
  void runSRT(std::list<int>& queue, Process* p);
  // run the PWA algorithm
  void runPWA(std::list<int>& queue, Process* p);
  // get FCFS stats
  long double getFCFSWaitTime() const {
    return avg_wait_time_fcfs;
  }
  long double getFCFSBurstTime() const {
    return avg_burst_time_fcfs;
  }
  long double getFCFSTurnAroundTime() const {
    return avg_trn_arnd_time_fcfs;
  }
  int getFCFSContextSwitches() const {
    return num_fcfs_contexts;
  }

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

  // get PWA stats
  long double getPWAWaitTime() const {
    return avg_wait_time_pwa;
  }
  long double getPWABurstTime() const {
    return avg_burst_time_pwa;
  }
  long double getPWATurnAroundTime() const {
    return avg_trn_arnd_time_pwa;
  }
  int getPWAContextSwitches() const {
    return num_pwa_contexts;
  }


 private:

  // context change time
  int t_cs;
  // number of processes
  int n;
  int num_fcfs_bursts;
  int num_srt_bursts;
  int num_pwa_bursts;
  int num_fcfs_contexts;
  int num_srt_contexts;
  int num_pwa_contexts;
  long double avg_burst_time_fcfs;
  long double avg_burst_time_srt;
  long double avg_burst_time_pwa;
  long double avg_wait_time_fcfs;
  long double avg_wait_time_srt;
  long double avg_wait_time_pwa;
  long double avg_trn_arnd_time_fcfs;
  long double avg_trn_arnd_time_srt;
  long double avg_trn_arnd_time_pwa;
  
}; 
