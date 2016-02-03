#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define BUFF_SIZE 1024

FILE* file;  // input file with expression

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
        // if x[i] is another nested expression
        if(x[i] == '(' && i > 0) {
            // increment count
            count++;
            int par_count = 1;
            while(par_count > 0 && i < n) {
                if(x[i] == '(') // count up for an opening brace
                    par_count++;
                else if(x[i] == ')')
                    par_count--; // count down for a closing brace
                i++;
            }
        }
        // if x[i] is a number
        else if(isdigit(x[i])) {
            count++;
            while(isdigit(x[i])) {
                i++;
            }
        }
    }

    return count;
}

/* function which evaluates an expression */
// l -> start (left) index of the string
// r -> end (right) index of the string
// x -> expression , the_parent -> process id of the root node
int compute(char* x, int l, int r, int the_parent) {
    
    int i; // running index
    int result = 0; // initialize the answer
    char op; // operator type
    int num_oper = num_operands(x, (r-l)); // number of operands

    for(i = l; i < r; i++) {
        // unknown operator
        if(x[i] != ' ' && is_operator(x[i]) == 0 && isdigit(x[i]) == 0
           && x[i] != '(' && x[i] != ')') {
            if(x[i] == '-' && isdigit(x[i+1]))
                continue;
            else {
                // store the unknown operator
                char arr[BUFF_SIZE];
                // running index
                int index = 0;
                while(x[i] != ' ' && i < r) {
                    arr[index] = x[i];
                    index++;
                    i++;
                }
                arr[index] = '\0';
                printf("PROCESS %d: unknown '%s' operator\n", getpid(), arr);
                exit(1);
            }
        }
        // if a character is an operator
        if(is_operator(x[i]) && x[i+1] == ' ') {
            // save operator
            op = x[i];
            // print msg
            printf("PROCESS %d: Starting '%c' operation\n", getpid(), x[i]);
            // if the expression has less than two operands
            if(num_oper < 2) {
                // print msg
                printf("PROCESS %d: ERROR: not enough operands\n", getpid());
                // exit
                exit(1);
            }
        }
        // if the character is a digit
        if(isdigit(x[i])) {
            // check if the number is negative
            int is_neg = 0;
            if(i > 0 && x[i-1] == '-')
                is_neg = 1;
            // store number
            char arr[BUFF_SIZE];
            int index = 0;
            // for negative numbers add a minus sign
            if(is_neg == 1) {
                arr[index] = '-';
                index++;
            }
            // save all the digts of the number
            while(isdigit(x[i])) {
                arr[index] = x[i];
                i++;
                index++;
            }
            arr[index] = '\0';
            // read and write ends of the pipe
            int p[2];
            int rc = pipe(p);
            // if the pipe fails
            if(rc == -1) {
              perror("pipe() failed\n");
              exit(1);
            }
            // fork another child process
            int pid = fork();
            if(pid == -1) {
              perror("fork() failed\n");
              exit(1);
            }
            // if in the child process
            if(pid == 0) {
              close(p[0]);
              p[0] = -1;
              // print msg
              printf("PROCESS %d: Sending '%s' on pipe to parent\n", getpid(), arr);
              // send data to parent
              int bytes_w = write(p[1], arr, index);
              // exit child process
              _exit(EXIT_SUCCESS);
            }
            // if in the parent process
            else {
              // wait for the child process to complete
              int status;
              pid_t child_pid = wait(&status);
              if(WIFSIGNALED(status))
                    printf("PARENT: child %d terminated abnormally\n", (int)child_pid);
              else if(WEXITSTATUS(status) != 0)
                    printf("PARENT: child %d terminated with nonzero exit status %d\n",
                           (int)child_pid, WEXITSTATUS(status));
              // close the write end of the pipe
              close(p[1]);
              p[1] = -1;
              // store the red data
              char arr[BUFF_SIZE];
              int bytes_r = read(p[0], arr, BUFF_SIZE);
              // update the result value
              arr[bytes_r] = '\0';
              if(result == 0)
                 result = atoi(arr);
              else {
                 if(op == '+')
                   result += atoi(arr);
                 if(op == '-')
                   result -= atoi(arr);
                 if(op == '*')
                   result *= atoi(arr);
                 if(op == '/')
                   result /= atoi(arr);
              }
            }
        }
        // if a subexpression is encountered
        else if(x[i] == '(' && i != l) {
            // store the subexpression in a string
            int index = 0;
            char sub_exp[BUFF_SIZE];
            int j = i;
            while(x[j] != ')') {
              sub_exp[index] = x[j];
              j++;
              index++;
            }
            i = j;
            sub_exp[index] = x[j];
            sub_exp[index+1] = '\0';
            // read and write ends of the pipe
            int p[2];
            int rc = pipe(p);
            if(rc == -1) {
              perror("pipe() failed\n");
              exit(1);
            }
            // fork a child process
            int pid = fork();
            if(pid == -1) {
              perror("fork() failed\n");
              exit(1);
            }
            // if in the child process
            if(pid == 0) {
              // close the read end of the pipe
              close(p[0]);
              p[0] = -1;
              // evaluate the subexpression
              int ans = compute(sub_exp, 0, index, the_parent);
              // print msg
              printf("PROCESS %d: Sending '%d' on pipe to parent\n", getpid(), ans);
              char arr[BUFF_SIZE];
              sprintf(arr, "%d", ans);
              // pipe to parent
              int bytes_w = write(p[1], arr, strlen(arr));
              // quit child process
              _exit(EXIT_SUCCESS);
            }
            // if in the parent process
            else {
              // wait for child process
              int status;
              pid_t child_pid = wait(&status);
              if(WIFSIGNALED(status))
                    printf("PARENT: child %d terminated abnormally\n", (int)child_pid);
              else if(WEXITSTATUS(status) != 0)
                    printf("PARENT: child %d terminated with nonzero exit status %d\n",
                           (int)child_pid, WEXITSTATUS(status));
              // close write end of the pipe
              close(p[1]);
              p[1] = -1;
              char arr[BUFF_SIZE];
              int bytes_r = read(p[0], arr, BUFF_SIZE);
              arr[bytes_r] = '\0';
              if(result == 0)
                result = atoi(arr);
              else {
                if(op == '+')
                  result += atoi(arr);
                if(op == '-')
                  result -= atoi(arr);
                if(op == '*')
                  result *= atoi(arr);
                if(op == '/')
                  result /= atoi(arr);
              }
            }
        }
    }
    // print the final answer
    if(getpid() == the_parent)
        printf("PROCESS %d: Final answer is '%d'\n", getpid(), result);
    return result;
    
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
  size_t len = strlen(line);
  // get the id of the first parent process
  int the_parent = getpid();
  // call the recursive function for computation
  int ans = compute(line, 0, len-1, the_parent);
  // end
  return EXIT_SUCCESS;

}
