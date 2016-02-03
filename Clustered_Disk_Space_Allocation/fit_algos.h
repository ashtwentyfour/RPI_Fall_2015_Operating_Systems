#include <map>
#include <list>


/* parent class FitAlgo */
class FitAlgo {

 public:
  // initialize variables
  FitAlgo(int n_blocks) {
    ID = 'A';
    num_clus = 0;
    y_ = n_blocks/4;
    x_ = 4;
    num_free = n_blocks;
    // initialize the memory block values
    for(int i = 0; i < x_; i++)
      for(int j = 0; j < 100; j++)
        memory[i][j] = '.';
  }
  // virtual function implemented differently for each algorithm
  virtual void allocate_memory(std::string s, int mem) {}
  // return number of free memory units
  int freeMem() const { return num_free; }
  // number of files currently in memory
  int numFile() const { return files.size(); }
  // number of clusters
  int numClus() const { return num_clus; }
  // get latest file ID used
  char latestID() const { return ID; }
  // check whether file exists
  bool fileExists(std::string f) const {
    if(files.find(f) != files.end())
      return true;
    return false;     
  }
  // get mem occupied by the file
  int getMem(const std::string f) const {
    if(files.find(f) != files.end()) {
      int mem = file_mem.at(f);
      return mem;
    }
    else
      return 0;
  }
  // get the ID (block type) of a file 
  char getID(const std::string f) const {
    if(files.find(f) != files.end()) {
      char ID = files.at(f);
      return ID;
    }
    else
      return '\0';
  }
  // print out memory
  void print() const {
    for(int i = 0; i < y_; i++)
      std::cout<<"=";
    std::cout<<std::endl;
    for(int i = 0; i < x_; i++) {
      for(int j = 0; j < y_; j++) {
	std::cout<<memory[i][j];
      }
    std::cout<<std::endl;
    }
    for(int i = 0; i < y_; i++)
      std::cout<<"=";
    std::cout<<std::endl;;
  }
  // display the number and list of files on the disk
  std::string dir() {
    std::string dir = "";
    std::string size = std::to_string(recents.size());
    dir += size + '\n';
    recents.sort();
    std::list<std::string>::iterator it;
    for(it = recents.begin(); it != recents.end(); it++) 
      dir += *it + '\n';
    return dir;
  }
  // deallocate memory when a file gets deleted 
  void deallocate(std::string s) {

    char id = files[s];
    // locate file in the memory
    int x = file_locs[s].first;
    int y = file_locs[s].second;
    // get process memory units
    int m = file_mem[s];
    // counter 
    int count = 0;
    int i = x;
    int j = y;
    // remove file from memory
    while(count < m) {
      if(i == x_)
        break;
      if(memory[i][j] == id) {
         memory[i][j] = '.';
         count++;
       }
      j++;
      if(j == y_) {
	i++;
        j = 0;
      }
    }
    // update amount of free space
    num_free += m;
    // remove file from the lists
    file_locs.erase(s);
    file_mem.erase(s);
    recents.remove(s);
    files.erase(s);
    updateNumClusters();
    if(files.size() == 0)
        ID = 'A';       
    return;
    
  }

  
 protected:

  char memory[4][100];  // memory matrix
  int x_;  // x memory limit
  int y_;  // y memory limit
  int num_free; // amount of free memory
  // file -> file location mapping
  std::map<std::string, std::pair<int,int> > file_locs;
  // file -> file memory mapping
  std::map<std::string,int> file_mem;
  // list of all the files in the system
  std::list<std::string> recents;
  // mapping of file name -> ID
  std::map<std::string,char> files;
  // latest file ID
  char ID;
  // number of clusters 
  int num_clus;
  // update the number of clusters
  void updateNumClusters() {
    int i = 0;
    int j = 0;
    int num = 0;
    while(i < x_) {
      if(memory[i][j] != '.') {
	num++;
        while(memory[i][j] != '.') {
          if(i == x_)
	    break;
	  j++;
          if(j == y_) {
            i++;
            j = 0;
          }
        }
      }
      j++;
      if(j >= y_) {
	j = 0;
        i++;
      }
    }
    num_clus = num;
  }

};



class ClusteredFit : public FitAlgo {

 public:
  // constructor
  ClusteredFit(int n_blocks) : FitAlgo(n_blocks) {}
  // allocate memory for a new file s
  void allocate_memory(std::string s, int mem) {
   
    // FILE ALREADY EXISTS
    int i = 0;
    int j = 0;
    int start_x, start_y;
    int size = 0;
    int flag = 0;  
    while(size < mem) {
      if(i == x_)
	break;
      if(memory[i][j] == '.') {
        if(flag == 0) {
	  start_x = i;
          start_y = j;
          flag = 1;
        }
	memory[i][j] = ID; 
        size++;
      }
      j++;
      if(j == y_) {
	i++;
        j = 0;
      }
    }

    // if (size < mem) -> NOT ENOUGH SPACE
    files[s] = ID;
    file_mem[s] = mem;
    recents.push_back(s);   
    file_locs[s] = std::make_pair(start_x,start_y);        
    updateNumClusters();
    if(ID == 'z') 
      ID = 'A';
    else
      ID++; 
  }

};  


