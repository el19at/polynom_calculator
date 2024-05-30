/*
Elya Athlan 318757200
polynom value calculator
compile: gcc ex4b.c -o ex4b 
run: ./ex4b
input: polynom, x (string)
accepted symbol: '+'(addition), '^'(power), digits(integer only), ','(to delimit polynom from x), x(the desired value) 
*/
#define N 512

#include <stdio.h>

#include <string.h>

#include <stdlib.h>

#include <sys/types.h>

#include <sys/shm.h>

#include <sys/wait.h>

#include <unistd.h>

double power(double, int);
int factor(char * , int * );
void get_args(char * expretion, int * factor, int * deg);
int eval_single_degree(char * , int);
int get_size(char * );
char ** str_to_array(const char * , int);
void free_str_array(char ** , int);
void input_str(char * );
int get_point_to_evaluate(char * );
int have_x(char * );

struct Args {
  char expr[N + 1];
  int x;
  int val;
}
Args;

int main() {
  while (1) {
    int sum = 0; //global var for final result
    char input[N + 1]; //init string for user input
    input_str(input); //get user input
    if (!strcmp(input, "done")) //end the program when "done" typed
      break;
    int size = get_size(input); //get the size of the polinoms(member to calc)
    char ** segm = (char ** ) malloc(sizeof(char ** ) * size); //init array of string for each member
    if (segm == NULL) {
      perror("malloc");
      exit(1);
    }
    segm = str_to_array(input, size); //split the input to segement to calc
    int x = get_point_to_evaluate(input); //extract the value of x from the user input

    struct Args * args; //init structs for shared memmory 
    struct shmid_ds shm_desc;
    //get shared memmory
    int key = ftok("/tmp", 'a');

    int shm_id = shmget(key, sizeof(Args) * size, IPC_CREAT | IPC_EXCL | 0600);
    if (shm_id < 0) {
      perror("shmget");
      continue;
    }

    void * smh_addr = shmat(shm_id, NULL, 0);
    if (!smh_addr) {
      perror("shmat");
      continue;
    }
    args = (struct Args * ) smh_addr;
    //iterate over the ploynom parts and calc on diffrant process
    for (int i = 0; i < size; i++) {
      strcpy(args[i].expr, segm[i]);
      args[i].x = x;
      pid_t pid = fork();
      if (pid < 0) {
        perror("fork");
        continue;
      }
      if (pid == 0) {
        //link the child process to the shared memmory
        shm_id = shmget(key, 0, 0600);
        if (shm_id < 0) {
          perror("shmget");
          continue;
        }

        smh_addr = shmat(shm_id, NULL, 0);
        if (!smh_addr) {
          perror("shmat");
          continue;
        }
        args = (struct Args * ) smh_addr;

        args[i].val = eval_single_degree(args[i].expr, x);
        exit(0);
      }
    }
    //wait for all process 
    int terminated = 0;
    while (terminated < size) {
      wait(NULL);
      terminated++;
    }
    //addition of the results
    for (int i = 0; i < size; i++)
      sum += args[i].val;

    //free shared memmory
    if (shmdt(smh_addr) < 0) {
      perror("shmdt");
      exit(1);
    }

    if (shmctl(shm_id, IPC_RMID, & shm_desc)) {
      perror("shmctl");
      exit(1);
    }
    //remove the x value from user input to preapre for the print
    for (int i = 0; i < strlen(input); i++)
      if (input[i] == ',') {
        input[i] = '\0';
        break;
      }
    printf("%s = %d\n", input, sum);
    sum = 0;
    free_str_array(segm, size);
  }
  return 0;
}
/**
 * It returns the value of base raised to the power of expo.
 * 
 * @param base The base of the number.
 * @param expo The exponent of the power function.
 * 
 * @return the value of the base raised to the power of the exponent.
 */
double power(double base, int expo) {
  if (base == 0)
    return 0;
  int res = 1;
  for (int i = 0; i < expo; i++)
    res *= base;
  return res;
}
/**
 * It takes a string and an integer pointer as input, and returns an integer. The integer pointer is
 * used to store the index of the first character of the next token
 * 
 * @param input The string that contains the input.
 * @param ind the index of the first character of the next term
 * 
 * @return The multiplicator of x.
 */
int factor(char * input, int * ind) {
  int flag = 0;
  * ind = 2;
  for (int i = 0; i < strlen(input); i++)
    if (input[i] == '*')
      flag = 1, * ind = i + 3;
  if (flag == 0)
    return 1;
  return atoi(input);
}
/**
 * It reads a string from the user and removes the '\n' character from the end of the string
 * 
 * @param a the string to be inputted
 */
void input_str(char * a) {

  fgets(a, N + 1, stdin);

  a[strlen(a) - 1] = '\0'; //remove the '\n' char from the string

}

/**
 * It takes a string and two integers as arguments, and it returns the factor and degree of the string
 * 
 * @param input the string that contains the factor and the degree
 * @param fact the factor of the term
 * @param deg degree of the polynomial
 * 
 * @return the factor and the degree of the polynomial.
 */
void get_args(char * input, int * fact, int * deg) {
  int index = 0;
  * fact = factor(input, & index);
  if (input[index - 1] != '^') { //x with no exponant 
    * deg = 1;
    return;
  }
  char s_deg[N];
  int i;
  for (i = index; i < strlen(input); i++) //get the last part of the string after the '^'
    s_deg[i - index] = input[i];
  s_deg[i - index] = '\0';
  * deg = atoi(s_deg);

}
/**
 * It returns 1 if the input string contains an 'x', and 0 otherwise
 * 
 * @param input the string to be checked
 * 
 * @return the value of the variable i.
 */
int have_x(char * input) {
  for (int i = 0; i < strlen(input); i++)
    if (input[i] == 'x')
      return 1;
  return 0;
}
/**
 * It takes a string and an integer, and returns the value of the string when the variable is replaced
 * by the integer
 * 
 * @param input the string to be evaluated
 * @param x the value of x to evaluate the polynomial at
 * 
 * @return The value of the polynomial at x.
 */
int eval_single_degree(char * input, int x) {
  if (!have_x(input)) //no x in the expretion 
    return atoi(input);
  int fact, deg;
  get_args(input, & fact, & deg);
  return fact * power(x, deg);
}
/**
 * It counts the number of times the character '+' appears in the input string
 * 
 * @param input the string that contains the input
 * 
 * @return The number of elements to calc in the input string.
 */
int get_size(char * input) {
  int counter = 1;
  for (int i = 0; i < strlen(input); i++)
    if (input[i] == '+')
      counter++;
  return counter;
}

/**
 * The function gets a string and a size and returns an array of strings
 * 
 * @param str The string to be split.
 * @param size the number of words in the string
 * 
 * @return an array of strings.
 */
char ** str_to_array(const char * str, int size) {
  char ** res;
  res = (char ** ) malloc(sizeof(char * ) * (size));
  if (res == NULL) {
    perror("malloc\n");
    exit(1);
  }
  char a[strlen(str) + 1]; //copy the original string for strtok() func
  strcpy(a, str);
  for (int i = 0; i < strlen(a); i++)
    if (a[i] == ',')
      a[i] = '\0';
  int i = 0; //init index for the array
  //iterate over the string allocate dynamicly string for each word and strore the word
  char * token = strtok(a, "+");
  while (token != NULL) {
    res[i] = (char * ) malloc(sizeof(char) * (strlen(token) + 1));
    if (res[i] == NULL) {
      free_str_array(res, i + 1); //free all the previous allocations 
      perror("malloc\n");
      exit(1);
    }
    strcpy(res[i], token);
    i++;
    token = strtok(NULL, "+");
  }
  return res;
}
/**
 * It takes a 2D array of strings and frees it
 * 
 * @param to_free the array of strings to free
 * @param size the number of strings in the array
 */
void free_str_array(char ** to_free, int size) {
  for (int i = 0; i < size; i++)
    free(to_free[i]);
  free(to_free);
}

/**
 * It takes a string as input and returns the point to evaluate the function at
 * 
 * @param input The input string
 * 
 * @return The point to evaluate the function at.
 */
int get_point_to_evaluate(char * input) {
  int flag = 0, ind;
  for (int i = 0; i < strlen(input); i++)
    if (input[i] == ',')
      flag = 1, ind = i + 1;
  if (!flag)
    return 1;
  char a[N];
  int i;
  for (i = ind; i < strlen(input); i++)
    a[i - ind] = input[i];
  a[i - ind] = '\0';
  return atoi(a);
}
