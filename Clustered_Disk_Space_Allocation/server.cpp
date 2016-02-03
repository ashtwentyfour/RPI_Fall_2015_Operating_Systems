#include <iostream>
#include <fstream>
#include <map>
#include <cmath>
#include <cstdlib>
#include <string>
#include <signal.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <cstdlib>

#include "fit_algos.h"

using namespace std;

#define BUF_SIZE 64
#define BACKLOG 8

FitAlgo* clustered;
int BLOCKSIZE;
// standard commands and error messages
const std::string store("STORE");
const std::string readfile("READ");
const std::string del("DELETE");
const std::string dir("DIR");
const std::string file_exists("ERROR: FILE EXISTS");
const std::string no_such_file("ERROR: NO SUCH FILE");
const std::string insuff_disk_space("ERROR: INSUFFICIENT DISK SPACE");
const std::string incon_size("ERROR: SPECIFIED SIZE DOES NOT MATCH SIZE OF CONTENT");
const std::string invalid_byte_range("ERROR: INVALID BYTE RANGE");

/* extract the size and the file name from the client command */
std::string process_storeinp(int& size, std::string inp, bool del) {
  // get file name
  int start = 0;
  std::string file_name = "";
  int num_spaces = 0;
  while(start < inp.size()) {
    
    if(inp[start] == ' ') {
      num_spaces++;
    }
    if(num_spaces == 2) 
      break;
    if(num_spaces == 1 && inp[start] != ' ' && inp[start] != '\n')
      file_name += inp[start];
    start++;

  }
  if(del)  // if the command is a delete command 
    return file_name;
  // else proceed to get the size of the file 
  start++;
  size = 0;
  std::string num_bytes = "";
  int index = start;
  while(index < inp.size()) {
    if(inp[index] == '\n')
      break;
    num_bytes += inp[index];
    index++;
  }
  // return the file name
  size = atoi(num_bytes.c_str());
  return file_name;
   
}

/* extract the file name, offset size, and the size of the read data */
std::string process_readinp(int& size, int& offset, std::string inp) {
  // first extract the file name
  int start = 0;
  int num_spaces = 0;
  std::string file_name = "";
  while(start < inp.size()) {
    if(inp[start] == ' ') {
      num_spaces++;
      if(num_spaces == 2) 
	break;
    }
    if(num_spaces == 1 && inp[start] != ' ' && inp[start] != '\n') {
      file_name += inp[start];
    }
    start++; 
  }
  // get the offset value 
  start++;
  std::string byte_offset = "";
  while(start < inp.size()) {
    if(inp[start] == ' ')
      break;
    byte_offset += inp[start];
    start++;
  }
  offset = atoi(byte_offset.c_str());
  start++;
  // get the size (bytes) of data to be read
  std::string num_bytes = "";
  while(start < inp.size()) {
    if(inp[start] == '\n')
      break;
    num_bytes += inp[start];
    start++;
  }
  size = atoi(num_bytes.c_str());
  return file_name;
}


void setup_sa_nocldwait() {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));

  /* continue to use the default SIGCHLD handler */
  sa.sa_handler = SIG_DFL;
  /* don't turn children into zombies */
  sa.sa_flags = SA_NOCLDWAIT;

  if (sigaction(SIGCHLD, &sa, NULL) != 0) {
    perror("sigaction");
    fprintf(stderr, "warning: failed to set SA_NOCLDWAIT\n");
  }
}

int setup_listeners(int sockets[], int n_sockets, int first_port) {
  struct sockaddr_in6 bindaddr;

  /* set common fields in bindaddr */
  memset(&bindaddr, 0, sizeof(bindaddr));
  bindaddr.sin6_family = AF_INET6;
  memcpy(&bindaddr.sin6_addr, &in6addr_any, sizeof(in6addr_any));

  for (int i = 0; i < n_sockets; i++) {
    /* create socket */
    sockets[i] = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockets[i] == -1) {
      perror("socket");
      return -1;
    }

    /* bind socket to port */
    bindaddr.sin6_port = htons(first_port + i);
    if (bind(
	     sockets[i],
	     (struct sockaddr *) &bindaddr,
	     sizeof(bindaddr)
	     ) != 0) {
      perror("bind");
      return -1;
    }

    /* start listening */
    if (listen(sockets[i], BACKLOG) != 0) {
      perror("listen");
      return -1;
    }
  }

  return 0;
}


// handle client requests until client terminates the connection
void *connection_handler(void *socket_desc)
{
  //Get the socket descriptor
  int sock = *(int*)socket_desc;
  int read_size;
  char *message;
  // allocate memory to receive messages
  char* client_message = (char*)malloc(sizeof(char)*524388);
  std::cout<<"Received incoming connection from "<<pthread_self()<<std::endl;
     
  // Receive a message from client
  while( (read_size = recv(sock , client_message , 524388 , 0)) > 0 ) {
      //end of string marker
      client_message[read_size] = '\0';
      // record message      
      std::string command(client_message);
      // name of file involved 
      std::string file_name = "";
      memset(client_message,0,524388);
      // if the command is a STORE file command 
      if(command.find(store) == 0) {
        read_size = recv(sock , client_message , 524388, 0);
	client_message[read_size] = '\0';
        // file content/data/bytes
	std::string content(client_message);
        memset(client_message,0,524388);
        int size = 0; // initialize size of the file
        // extract size and file name 
	file_name = process_storeinp(size,command, false);
        // compute the number of blocks required
        int mem = ceil(double(size)/BLOCKSIZE);
        // if the file exists
        if (clustered->fileExists(file_name)) {
          write(sock, (file_exists + '\n').c_str(), file_exists.size()+1);
	  std::cout<<"[thread "<<pthread_self()<<"] Sent: "<<file_exists<<std::endl;
	  continue;
        }
        // if enough disk space is not available 
        else if(clustered->freeMem() < size) { 
          write(sock,(insuff_disk_space + '\n').c_str(),insuff_disk_space.size()+1);
	  std::cout<<"[thread "<<pthread_self()<<"] Sent: "<<insuff_disk_space<<std::endl;
          continue;
        }
        // if the number of bytes and the actual size of the content are inconsistent 
        else if(size + 1 != content.size()) {
          write(sock, (incon_size + '\n').c_str(), incon_size.size()+1);
	  std::cout<<"[thread "<<pthread_self()<<"] Sent: "<<incon_size<<std::endl;
          continue;
        }
        // store file and it it to the list of stored files 
	std::cout<<"[thread "<<pthread_self()<<"] Rcvd: STORE "<<file_name<<" "<<size<<std::endl;
        clustered->allocate_memory(file_name,mem);  // allocate memory 
	std::cout<<"[thread "<<pthread_self()<<"] Stored file '"<<clustered->getID(file_name)<<"' ("<<size<<" bytes; "<<
	  mem<<" blocks; "<<clustered->numClus();
        if(clustered->numClus() > 1)
	  std::cout<<" clusters)"<<std::endl;
        else
	  std::cout<<" cluster)"<<std::endl;
        // print simulated memory blocks 
	std::cout<<"[thread "<<pthread_self()<<"] Simulated Clustered Disk Space Allocation:"<<std::endl;
        clustered->print();
	std::cout<<"[thread "<<pthread_self()<<"] Sent: ACK"<<std::endl;
        // add a new file to .storage
	std::ofstream file;
	std::string loc = ".storage/" + file_name;
        file.open(loc);
        file<<content;
        file.close();        
        write(sock,"ACK\n", 4);
      } 
      // if a READ command is specified
      else if(command.find(readfile) == 0) {
        // initialize thr offset and size values 
        int size = 0;
        int off = 0;
        // extract file name, offset, and size 
	file_name = process_readinp(size,off,command);
	std::cout<<"[thread "<<pthread_self()<<"] Rcvd: READ "<<file_name<<" "<<off<<" "<<size<<std::endl;
        if(!clustered->fileExists(file_name)) {
          write(sock, (no_such_file + '\n').c_str(), no_such_file.size() + 1);
	  std::cout<<"[thread "<<pthread_self()<<"] Sent: "<<no_such_file<<std::endl;
          continue;
        } 
        int file_mem = BLOCKSIZE*clustered->getMem(file_name); // recover the actual file size (bytes)
        if(file_mem < size || off >= file_mem) {
	  std::cout<<"[thread "<<pthread_self()<<"] Sent: "<<invalid_byte_range<<std::endl;
          write(sock, (invalid_byte_range + '\n').c_str(), invalid_byte_range.size() + 1);
          continue;
        }
        // open file to get contents 
	std::string loc = ".storage/" + file_name;
	std::ifstream read_file(loc);
	std::string con = "";
	std::string line;
	if(read_file.is_open()) {
	  while(std::getline(read_file,line)) { 
	    con += line;
            con += '\n';
	  }
          read_file.close();
        }                            
        // apply offset 
        std::string read_con = "";
        int count = 0;
        for(int i = off; i < con.size() && count <= size; i++, count++)
	      read_con += con[i];
        // send acknowledgement message to the client 
        read_con = "ACK " + std::to_string(size) + '\n' + read_con;
        std::cout<<"[thread "<<pthread_self()<<"] Sent: ACK "<<size<<std::endl;
        std::cout<<"[thread "<<pthread_self()<<"] Sent "<<size<<" bytes (from "<<clustered->getMem(file_name)<<" '"<<
        clustered->getID(file_name)<<"' blocks) from offset "<<off<<std::endl;
        write(sock,read_con.c_str(),read_con.size()); 
      }
      // if the command is a DELETE command 
      else if(command.find(del) == 0) {
        int size = 0;
        // get file name and size 
	file_name = process_storeinp(size,command,true);
	std::cout<<"[thread "<<pthread_self()<<"] Rcvd: DELETE "<<file_name<<std::endl;
        // if the file does not exist 
        if(!clustered->fileExists(file_name)) {
	  std::cout<<"[thread "<<pthread_self()<<"] Sent: "<<no_such_file<<std::endl;
          write(sock,(no_such_file + '\n').c_str(), no_such_file.size()+1);
          continue;
        }
	std::cout<<"[thread "<<pthread_self()<<"] Sent: ACK"<<std::endl;
        // otherwise delete the file 
        char id = clustered->getID(file_name);
        int m_size = clustered->getMem(file_name);
        clustered->deallocate(file_name);  // deallocate memory 
	std::string del_cmd = "exec rm .storage/" + file_name;
        system(del_cmd.c_str()); // delete 
	std::cout<<"[thread "<<pthread_self()<<"] Deleted "<<file_name<<" file '"<<id<<"' (deallocated "<<
        m_size<<" blocks)"<<std::endl;
	std::cout<<"[thread "<<pthread_self()<<"] Simulated Clustered Disk Space Allocation:"<<std::endl;
	clustered->print();
        write(sock, "ACK\n", 4); // send ack
      }
      // if the command is a DIR command to see the list of files 
      else if(command.find(dir) == 0) {
        // get the list of files 
	std::string dir = clustered->dir();
        // send the list back 
        write(sock,dir.c_str(),dir.size());        
      }
    }
    // if the client disconnects 
    if(read_size == 0) {
      std::cout<<"[thread "<<pthread_self()<<"] Client closed its socket....terminating"<<std::endl;
      free(client_message);
      client_message = NULL;
      fflush(stdout);
    }
    else if(read_size == -1) {
      perror("recv failed");
    }
         
    return 0;
} 


void do_accept(int listen_sockfd) {
  int peer_sockfd;
  struct sockaddr_in6 src;
  socklen_t srclen;
  pthread_t thread_id;
  /* accept the connection */
  srclen = sizeof(src);
  while(true) {
    peer_sockfd = accept(listen_sockfd, (struct sockaddr *)&src, &srclen);
    if(peer_sockfd < 0)
       break;
	  if(pthread_create( &thread_id , NULL ,  connection_handler,
        (void*) &peer_sockfd) < 0) {
	      perror("could not create thread");
	      return;
	    }
	}
}


int main(int argc, char *argv[]) {
  /* check number of input arguements */
  if(argc != 4) {
    cerr << "ERROR: Incorrect number of arguements"<<endl;
    exit(0);
  }

  /* get blocksize */
  int blocksize = atoi(argv[2]);
  BLOCKSIZE = blocksize;
  cout<<"Block size is "<<blocksize<<endl;
  /* get number of blocks */
  int n_blocks = atoi(argv[3]);
  cout<<"Number of blocks is "<<n_blocks<<endl;
  /* get port no. */
  int port = atoi(argv[1]);

  /* create folder to store files */
  mkdir(".storage", 0777);
  std::ofstream(".storage/dummy.txt");
  system("exec rm .storage/*");
  
  int listen_sockfds[1];

  fd_set rfds;
  int maxfd = 0;
  // memory simuation
  ClusteredFit clus(n_blocks);
  clustered = &clus;
  
  /* Set up SIGCHLD handler with SA_NOCLDWAIT (option 3) */
  setup_sa_nocldwait();

  /* Set up our listening sockets. */
  setup_listeners(listen_sockfds, 1, port);
  
  /* Loop infinitely, accepting any connections we get. */
  for (;;) {
    /* Initialize our fd_set. */
    FD_ZERO(&rfds);
    maxfd = 0;
    FD_SET(listen_sockfds[0], &rfds);
    if (listen_sockfds[0] > maxfd) {
       maxfd = listen_sockfds[0];
     }

    /* Call select() and handle errors. */
    if (select(maxfd + 1, &rfds, NULL, NULL, NULL) == -1) {
      if (errno == EINTR) {
	continue;
      } else {
	perror("select");
	return 0;
      }
    }

    /* Iterate through fds, finding any that are ready. */
    if (FD_ISSET(listen_sockfds[0], &rfds)) {
      /* accept and fork the child process */
      do_accept(listen_sockfds[0]);
    }
  }
}
