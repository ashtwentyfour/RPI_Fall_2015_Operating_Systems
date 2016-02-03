#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>

#define BUFF_SIZE 1024

FILE* file;  // input file with expression

// global result variable
int results[BUFF_SIZE];

// thread ID of the main thread
long long int main_id;

// global thread id array
long long int threads[BUFF_SIZE];

// operators of each operand
char parent_op[BUFF_SIZE];

// global mutex variable 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// declaration of function to be executed by each thread
void* num(void* arg);


/* functions returns 1 when a character is an operator */
int is_operator(char x) {
    
    return x == '*' || x == '-' || x == '/'
      || x == '+';
}


/* number of operands in an expression */
int num_operands(char* x, int n) {
    
  int i;
  int count = 0;
  for(i = 0; i < n; i++) {
    // if x[i] is a number/integer
    if(isdigit(x[i])) {
      count++;
      while(isdigit(x[i+1]) && i < n) {
        i++;
      }
      continue;
    }
    // if x[i] is another nested expression
    if(x[i] == '(' && i > 0) {
      // increment count
      count++;
      int start = i;
      int par_count = 1;
      while(par_count > 0 && i < n) {
	if(x[i] == '(' && i > start) // count up for an opening brace
	  par_count++;
	else if(x[i] == ')')
	  par_count--; // count down for a closing brace
	i++;
      }
    }
  }

  return count;

}


// evaluate an expression
void compute(char* x, int* n, long long int* main_id) {
  
  int t;
  // flag = 1 after the first operand is multiplied for '*'
  int multi_flag = 0;
  // global index of the parent thread
  int parent_index;
  // parent thread ID
  long long int parent_id = (long long int)pthread_self();
  // get/assign parent thread global index
  for(t = 0; t < BUFF_SIZE - 1; t++) {
      if(threads[t] == parent_id) {
          parent_index = t;
          break;
      }
      else if(threads[t] != parent_id && threads[t+1] == -1) {
          parent_index = t + 1;
          threads[t+1] = parent_id;
          break;
      }
  }
  // count number of operands -> integers & sub-expressions
  int num_oper = num_operands(x, *n);
  // <= 1 operands
  if(num_oper <= 1) {
    printf("THREAD %d: ERROR: not enough operands\n", (int)pthread_self());
    exit(1);
  }
  int i, rc, res;
  // operator
  char op;
  // pthread ids
  pthread_t tid[BUFF_SIZE];
  //printf("Ashwin\n");
  // counter for the number of pthreads created
  int j = 0;
  // check for operators
  if(is_operator(x[1])) {
     op = x[1];
     printf("THREAD %d: Starting '%c' operation\n", 
       (int)pthread_self(), op);
  }
  // if a valid operator is not found
  else {
    // store unknown operator in a string
    char* uop = (char*)malloc((*n)*sizeof(char));
    int k = 1;
    int index = 0;
    while(x[k] != ' ' && k < *n) {
      uop[index] = x[k];
      k++;
      index++;
    }
    uop[index] = '\0';
    // print msg and exit
    printf("THREAD %d: ERROR: unknown '%s' operator\n", 
      (int)pthread_self(), uop);
    free(uop);
    exit(1);
  }
  
  // start evaluating expression 
  for(i = 3; i < *n; i++) {
    // if an integer operand is found (+ve or -ve)    
    if(isdigit(x[i]) || (x[i] == '-' && isdigit(x[i+1]))) {
      // allocate memory to hold the integer
      char* intg = (char*)malloc((*n)*sizeof(char));
      int index = 0;
      while(isdigit(x[i]) || (x[i] == '-' && isdigit(x[i+1]))) {
	    intg[index] = x[i];
        i++;
        index++;
      }
      // save operator to be passed to the thread
      intg[index] = op;
      index++;
      intg[index] = '\0';  
      // new thread to handle the integer
      rc = pthread_create(&tid[j], NULL, num, intg);
      if(rc != 0) 
        fprintf(stderr, "could not create thread (%d)\n", rc);
      // increase the count of the number of threads 
      j++;
    }
    // if a sub-expression is found
    else if(x[i] == '(') {
      int count = 1;
      int start = i;
      // allocate memory to store the expression 
      char* t = (char*)malloc((*n)*sizeof(char));
      int index = 0;
      // determine expression by counting the number of '('
      while(count != 0) {
        if(x[i] == '(' && i > start)
          count++;
        else if(x[i] == ')')
          count--;
        t[index] = x[i];
        i++;
        index++;
      }
      // save operator to be passed to the thread
      t[index] = op;
      index++;
      t[index] = '\0';
      // new thread to evaluate this expression
      rc = pthread_create(&tid[j], NULL, num, t);
      if(rc != 0)
        fprintf(stderr, "could not create thread (%d)\n", rc);
      // increment the count of number of threads
      j++;
    }
  }  

  // call join on each thread
  for(i = 0; i < num_oper; i++)  {
    
    // non-zer exit status of thread
    int join = pthread_join(tid[i], NULL);
    if(join != 0) {
       printf("THREAD %d: child %d terminated with nonzero exit status %d\n",
             (int)pthread_self(), (int)tid[i], join);
       exit(1);
    }
    // get global index of the thread
    int index;
    for(j = 0; j < BUFF_SIZE - 1; j++) {
        if(threads[j] == (long long int)tid[i]) {
            index = j;
            break;
        }
        else if(threads[j] != (long long int)tid[i] && threads[j+1] == -1) {
            index = j + 1;
            threads[j+1] = (long long int)tid[i];
            break;
        }
    }
    // update result of parent thread expression
    pthread_mutex_lock(&mutex);
    if(op == '+') {
        if(results[parent_index] == -1)
            results[parent_index] = 0;
        results[parent_index] += results[index];
    }
    else if(op == '-') {
        if(results[parent_index] == -1)
            results[parent_index] = results[index];
        else
            results[parent_index] -= results[index];
    }
    else if(op == '*') {
        if(results[parent_index] == -1 && multi_flag == 0) {
            results[parent_index] = results[index];
            multi_flag = 1;
        }
        else
            results[parent_index] *= results[index];
    }
    else if(op == '/') {
        if(results[parent_index] == -1)
            results[parent_index] = results[index];
        else {
            if(results[index] == 0) {
                fprintf(stderr, "ERROR: divide by zero condition\n");
                exit(1);
            }
            results[parent_index] /= results[index];
        }
    }
    pthread_mutex_unlock(&mutex);
      
  }

  // print msg
  printf("THREAD %d: Ended '%c' operation with result '%d'\n", 
	 (int)pthread_self(), op, results[parent_index]);
  
  // print msg regarding operation on sub-expression result
  if((int)pthread_self() != *main_id) {
      
      if(parent_op[parent_index] == '+') {
          printf("THREAD %d: Adding '%d'\n", (int)pthread_self(), results[parent_index]);
      }
      else if(parent_op[parent_index] == '-') {
          printf("THREAD %d: Subtracting '%d'\n", (int)pthread_self(), results[parent_index]);
      }
      else if(parent_op[parent_index] == '*') {
          printf("THREAD %d: Multiplying by '%d'\n", (int)pthread_self(), results[parent_index]);
      }
      else if(parent_op[parent_index] == '/') {
          printf("THREAD %d: Dividing by '%d'\n", (int)pthread_self(), results[parent_index]);
      }
  }

}



// return the integer or evaluate the sub expression
void* num(void* arg) {

  // get global index of the child thread
  int index = 0;
  int i;
  long long int id = (long long int)pthread_self();
  for(i = 0; i < BUFF_SIZE - 1; i++) {
      if(threads[i] == id) {
          index = i;
          break;
      }
      else if(threads[i+1] == -1 && threads[i] != id) {
          index = i+1;
          threads[i+1] = id;
          break;
      }
  }
  // integer value of the operand passed
  int* t = (int*)malloc(sizeof(int));
  char* arr = (char*)arg;
  // if the argument is an integer 
  if(isdigit(arr[0]) || (arr[0] == '-' && isdigit(arr[1]))) {
    // get the parent thread operator
    int len = strlen(arr);
    char op = arr[len-1];
    arr[len-1] = '\0';
    // convert to int
    *t = atoi(arr);
    // modify result
    results[index] = *t;
    // record operator from parent thread
    parent_op[index] = op;
    // print msg
    if(op == '+') {
        printf("THREAD %lld: Adding '%d'\n", id, *t);
    }
    else if(op == '-') {
        printf("THREAD %lld: Subtracting '%d'\n", id, *t);
    }
    else if(op == '*') {
        printf("THREAD %lld: Multiplying by '%d'\n", id, *t);
    }
    else if(op == '/') {
        printf("THREAD %lld: Dividing by '%d'\n", id, *t);
    }
    // release dynamically allocated memory
    free(t);
    free(arr);
  }
  // if the argument is a sub-expression
  else {
    // get the operator from the parent thread
    int len = strlen(arr);
    char op = arr[len-1];
    parent_op[index] = op;
    arr[len-1] = '\0';
    // evaluate the expression using threads
    len--;
    compute((char*)arr, &len, &main_id);
    // free dynamically allocated memory
    free(arr);
    free(t);
  } 
  
  return NULL;

} 
  

int main(int argc, char* argv[]) {

  // program requires two command line arguments
  if(argc != 2) {
    fprintf(stderr, "ERROR: program requires two arguments\n");
    fprintf(stderr, "USAGE: %s %s \n", argv[0], argv[1]);
    return EXIT_FAILURE;
  }
  // file with the expression
  file = fopen(argv[1], "r");
  if(!file) {
    perror(argv[1]);
    return EXIT_FAILURE;
  }
  // each line in the file
  char line[BUFF_SIZE];
  int flag = 0;
  while(fgets(line, BUFF_SIZE, file) != NULL) {
    if(line[0] != '#' && line[0] == '(') {
      flag = 1;
      break;
    }
  }
  // if no expressions were found
  if(flag == 0) {
    fprintf(stderr, "ERROR: no valid expression found!");
    return EXIT_FAILURE;
  }
  // close file
  fclose(file);
  // length of expression string
  size_t l = strlen(line);
  // integer length
  int len = (int)l;
  // initialize global arrays
  int j;
  for(j = 0; j < BUFF_SIZE; j++) {
        results[j] = -1;
        threads[j] = -1;
        parent_op[j] = '$';
  }
  // record ID of the parent/main thread
  threads[0] = (long long int)pthread_self();
  main_id = threads[0];
  // evaluate expression
  compute(line, &len, &main_id);
  // display final answer
  printf("THREAD %d: Final answer is '%d'\n", (int)pthread_self(), results[0]);

  return EXIT_SUCCESS;


}
