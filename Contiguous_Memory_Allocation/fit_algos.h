#include <map>
#include <list>
#include "partition.h"


/* parent class FitAlgo */
class FitAlgo {

 public:
  // initialize variables
  FitAlgo(int X_, int Y_) { 
    defrag = false; 
    x_ = X_;
    y_ = Y_;
    num_free = X_*Y_;
    // allocate x*y units of memory 
    memory = new char*[x_];
    for(int i = 0; i < x_; i++)
      memory[i] = new char[y_];
    for(int i = 0; i < x_; i++)
      for(int j = 0; j < y_; j++)
        memory[i][j] = '.';
  }
  // virtual destructor
  virtual ~FitAlgo() {}
  // virtual function implemented differently for each algorithm
  virtual void allocate_memory(char x, int mem) {}
  // return number of free memory units
  int freeMem() const { return num_free; }
  // number of processes currently in memory
  int numProc() const { return processes.size(); }
  // whether or not defragmentation is required
  bool defragReq() const { return defrag; }
  // print out memory
  void print() const {
    for(int i = 0; i < y_; i++)
      std::cout<<'=';
    std::cout<<std::endl;
    for(int i = 0; i < x_; i++) {
      for(int j = 0; j < y_; j++) {
	std::cout<<memory[i][j];
      }
      std::cout<<std::endl;
    }
    for(int i = 0; i < y_; i++)
      std::cout<<'=';
    std::cout<<std::endl;
  }

  // deallocate memory after a process has ended
  void deallocate(char proc) {

    // locate process in the memory
    int x = processes[proc].first;
    int y = processes[proc].second;
    // get process memory units
    int m = process_mem[proc];
    // counter 
    int count = 0;
    int i = x;
    int j = y;
    // remove process from memory
    while(count < m) {
      if(i == x_)
        break;
      memory[i][j] = '.';
      count++;
      j++;
      if(j == y_) {
	i++;
        j = 0;
      }
    }
    // update amount of free space
    num_free += m;
    // remove process from the lists
    processes.erase(proc);
    process_mem.erase(proc);
    recents.remove(proc);

    return;
    
  }

  // function to carry out defragmentation
  void defragment(int& num_moved) {
  
    // create o list of all the processes and clear the memory
   char current;
   std::list<char> proc_list;
   for(int i = 0; i < x_; i++) { 
     for(int j = 0; j < y_; j++) { 
       if(memory[i][j] != '.' && memory[i][j] != current) {
	 proc_list.push_back(memory[i][j]);
         current = memory[i][j];
       }
       memory[i][j] = '.';
     }
   }

   // reallocate the memory
  int x = 0, y = 0, p, q, count = 0, M;
  // for each process
  for(std::list<char>::iterator it = proc_list.begin(); 
        it != proc_list.end(); it++) {
    // get memory units 
    M = process_mem[*it];
    p = x;
    q = y;
    // move memory
    while(count < M) {
      if(p == x_)
	break;
      count++;
      memory[p][q] = *it;
      q++;
      if(q == y_) {
	q = 0;
        p++;
      }
    }
    // if the process has been shifted
    if(x != processes[*it].first || y != processes[*it].second) {
      num_moved += M;
      processes[*it] = std::make_pair(x,y);
    }
    x = p;
    y = q;
    count = 0;      
  }
  // defragmentation complete
  defrag = false;
 
  return;

 }

 protected:

  char** memory;  // memory matrix
  int x_;  // x memory limit
  int y_;  // y memory limit
  int num_free; // amount of free memory
  bool defrag;  // defragmentation required
  // process ID -> process location mapping
  std::map<char, std::pair<int,int> > processes;
  // process ID -> process memory mapping
  std::map<char,int> process_mem;
  // list of all the processes in memory 
  std::list<char> recents;

};


/* First-Fit Algorithm */
class FirstFit: public FitAlgo {

 public:
   FirstFit(int X_, int Y_) : FitAlgo(X_,Y_) {} 
  // free dynamically allocated memory
  ~FirstFit() {
    for(int i = 0; i < x_; i++)
      delete [] memory[i];
    delete [] memory;
  }
  // first fit memory allocation method
  void allocate_memory(char x, int mem) {
   
    int count = 0;
    for(int i = 0; i < x_; i++) {
      for(int j = 0; j < y_; j++) {
        // if a partition is found
	if(memory[i][j] == '.') {
	  int p = i, q = j;
          // compute the size of the partition
          while(memory[p][q] == '.' && count < mem) {
            // increment memory count
            count++;
            q++;
            if(q == y_) {
	      q = 0;
              p++;
            }
            if(p == x_)
	      break;
          }
          int stop_x = p;
          int stop_y = q;
          // if a fit is found 
	  if(count == mem) {
            // add process
	    std::pair<int,int> pos;
	    pos = std::make_pair(i,j);
            process_mem[x] = mem;
            processes[x] = pos;
            // allocate memory
            p = i;
            q = j;
            count = 0;
            while(memory[p][q] == '.' && count < mem) {
	      // allocate memory
              memory[p][q] = x;
              count++;
              q++;
              if(q == y_) {
                q = 0;
                p++;
              }
              if(p == x_)
		break;
            }
            // update number of free memory units
            num_free -= mem;
            // add process
            recents.push_back(x);
            return;
	  }
          // if a fit is not found continue looking
	  else { 
            count = 0;
            i = stop_x;
            j = stop_y;
            if(i >= x_ || j >= y_)
	      break;
          }
        }
      }
    }
    // defragmentation required in case no fit is found
    defrag = true;
  }

};


/* Next-Fit Algorithm */
class NextFit: public FitAlgo {
    
 public:
   NextFit(int X_, int Y_) : FitAlgo(X_,Y_) {}
  // free dynamically allocated memory    
  ~NextFit() {
     for(int i = 0; i < x_; i++)
        delete [] memory[i];
     delete [] memory;
   }
  // next-fit memory allocation algorithm    
   void allocate_memory(char x, int mem) {
        
     int count = 0;
     int start_x = 0;
     int start_y = 0;
     // identify most recently added process
     if(recents.size() > 0) {
	std::list<char>::iterator it = --recents.end();
        char most_recent = *it;
        start_x = processes[most_recent].first;
        start_y = processes[most_recent].second;
      }
      // start from the most recently added process
      int j = start_y;
      int i = start_x;
      while(i < x_) {
	  // if a partition is found     
          if(memory[i][j] == '.') {
             int p = i;
             int q = j;
             // allocate memory similar to the First-Fit method
	     while(memory[p][q] == '.' && count < mem) {
	       // determine whether the process can be added 
	       count++;
	       q++;
	       if(q == y_) {
		 q = 0;
		 p++;
	       }
               if(p == x_)
		  break;
	     }
             int stop_x = p;
	     int stop_y = q; 
             // if the process can be added               
             if(count == mem) {
	        // include the process
                std::pair<int,int> pos;
                pos = std::make_pair(i,j);
                process_mem[x] = mem;
                processes[x] = pos;
                // allocate the memory
                p = i;
                q = j;
                count = 0;
		while(memory[p][q] == '.' && count < mem) {    
                  memory[p][q] = x;
		  count++;
		  q++;
		  if(q == y_) {
		    q = 0;
		    p++;
		  }
                  if(p == x_)
                    break;
		}
                // update amount of free memory                      
                num_free -= mem;
                recents.push_back(x);
                return;
	     }
             else {
                count = 0;
                i = stop_x;
                j = stop_y;
                continue;
             }
	   }
          j++;
	  if(j == y_) {
	    j = 0;
            i++;
          }
      }
        // if no fit is founf begin defragmentation 
        defrag = true;
   }

};


/* Best-Fit Algorithm */
class BestFit: public FitAlgo {

 public:
   BestFit(int X_, int Y_) : FitAlgo(X_,Y_) {}
  // free dynamically allocated memory
  ~BestFit() {
    for(int i = 0; i < x_; i++)
      delete [] memory[i];
    delete [] memory;
   }
  // best-fit memory allocation
  void allocate_memory(char x, int mem) {

    int count = 0;
    // if no processes have been allocated yet
    if(recents.size() == 0) {
      recents.push_back(x);
      int i = 0,j = 0;
      // allocate memory from x = 0, y = 0
      while(count < mem) {
        if(i == x_)
          break;
        count++;
        memory[i][j] = x;
        j++;
        if(j == y_) {
	  j = 0;
          i++;
        }
      }
      // add process
      processes[x] = std::make_pair(0,0);
      process_mem[x] = mem;
      return;
    }
    // identify all the partitions
    std::list<MemPartition> partitions;
    int i,j;
    for(i = 0; i < x_; i++) {
      for(j = 0; j < y_; j++) {
        // if a partition is found
	if(memory[i][j] == '.') {
	  int count = 0;
          int p = i,q = j;
          int s_x = i;
          int s_y = j;
          // mark start and end of partition
	  while(memory[p][q] == '.') {
	    count++;
            if(p == x_ - 1 & q == y_ - 1)
	      break;
	    if(q == y_-1) {
	      q = 0;
	      p++;
	    }
            else
              q++;
            if(p == x_)
	      break;
	  }           
          int e_x = p;
          int e_y = q;
          // if an exact fir is found - allocate memory
          if(count == mem) {
            // counter
            int incr = 0;
            int r = s_x;
            int s = s_y; 
	    while(memory[r][s] == '.' && incr < mem) {
	      // allocate memory
	      memory[r][s] = x;
	      incr++;
	      s++;
	      if(s == y_) {
		s = 0;
		r++;
	      }
              if(r == x_)
                break;
	    }
            // add process
            processes[x] = std::make_pair(s_x, s_y);
            process_mem[x] = mem;
            recents.push_back(x);
            return;
	  }
          // if a bigger partition is found
          if(count > mem) {
            // add partition to the list of partitions
	    MemPartition P(s_x, s_y, e_x, e_y, count);
            partitions.push_back(P);
          }
          i = e_x;
          j = e_y;
          if(i >= x_ || j >= y_)
	    break;
	}
      }
    }
    // if no partitions were found - carry out defragmentation
    if(partitions.size() == 0) {
      defrag = true;
      return;
    }
    // sort the partitions in ascending order of size 
    partitions.sort();
    std::list<MemPartition>::iterator it = partitions.begin();
    // select the first partition and allocate memory
    int t = (*it).getSx();
    int u = (*it).getSy();
    count = 0;
    while(memory[t][u] == '.' && count < mem) {
      memory[t][u] = x;
      count++;
      u++;
      if(u == y_) {
	u = 0;
	t++;
      }
      if(t == x_)
        break;
    }
    // add the process
    processes[x] = std::make_pair((*it).getSx(), (*it).getSy());
    process_mem[x] = mem;
    recents.push_back(x);
    return;        
   
  }


};





  


