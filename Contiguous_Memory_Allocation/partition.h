/* represents the memory partitoons */
class MemPartition {

 public:
  // initialize
  MemPartition(int s_x, int s_y, int e_x, int e_y, int s) {
    start_x = s_x;
    start_y = s_y;
    end_x = e_x;
    end_y = e_y;
    size = s;
  }

  int getSize() const { return size; }

  int getSx() const { return start_x; }

  int getSy() const { return start_y; }

  int getEx() const { return end_x; }

  int getEy() const { return end_y; }

  // compare two partitions by size and then by location
  bool operator<(const MemPartition& p2) const {
    if(getSize() == p2.getSize())
      return getSx() < p2.getSx();
    else
      return getSize() < p2.getSize();
  }

 private:
  // size and locations 
  int start_x; 
  int start_y;
  int end_x;
  int end_y;
  int size;

};
