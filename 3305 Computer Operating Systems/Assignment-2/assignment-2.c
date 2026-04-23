#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

/*
When compiling this code use -lm at the end of the command to link the math library
*/


int is_valid_integer(const char *str);
void parent_function(int n,int a, int b,int a1, int a2, int b1, int b2, int port[2][2]);
int parent_helper(int port[2][2],int a, int b, int size, pid_t pid);
void child_functon(int port[2][2]);
void child_helper(int port[2][2], pid_t pid, int size);
int int_length(int num);
void printHeader(char letter);
void print_parent_send(pid_t pid, int a, int b);
void print_parent_receive(pid_t pid, int result);
void print_child_send(pid_t pid, int result);
void print_child_receive(pid_t pid, int a, int b);



int main(int argc, char *argv[]){

    //verify that the parameters given meet the requirements
    if (argc != 3){
        printf("Not enough arguments, exiting.\n");
        return 1;
    }
    
    if(!is_valid_integer(argv[1]) || !is_valid_integer(argv[2])){
        printf("Invalid integer entered, exiting.\n");
        return 1;
    }

    //convert from int to string
    int a = atoi(argv[1]);
    int b = atoi(argv[2]);

    //check the parameters are in the specfied range
    if(a<1000 || a>9999 || b<1000 || b>9999){
        printf("The numbers you have entered are not valid, enter numbers from the range (1000 - 9999)\n");
        return 1;
    }

    //calculate the partitions
    int a1 = a/100;
    int a2 = a%100;
    
    int b1 = b/100; 
    int b2 = b%100;

    int port[2][2];

    //establish bidirectional pipes
    if (pipe(port[0]) < 0){
        perror("pipe error");
        exit(0);
    } 

    if (pipe(port[1]) < 0){
        perror("pipe error");
        exit(0);
    } 
    
    //create child process
    pid_t pid;
    if((pid = fork()) < 0){
        perror("fork error\n");
        exit(0);
    }

    if(pid==0){
        //child process
        child_functon(port);
    }
    else{
        //parent process
        printf("Your integers are %d %d\n",a,b);
        printf("Parent (PID %d): created child (PID %d)\n",getpid(),pid);
        int n = int_length(a);
        parent_function(n, a, b, a1, a2, b1, b2, port);
    }
    return 1;
}

void parent_function(int n, int a, int b, int a1, int a2, int b1, int b2, int port[2][2]){

    pid_t pid = getpid();
    int size = sizeof(int);
    int A, B, C, D, X, Y, Z;
    //close unused ends of the pipe
    close(port[0][0]);
    close(port[1][1]);

    printHeader('X');
    A = parent_helper(port,a1,b1,size,pid);
    //calculate X
    X = A* pow(10,n);

    printHeader('Y');
    B = parent_helper(port,a2,b1,size,pid);
    C = parent_helper(port,a1,b2,size,pid);
    //calculate Y
    Y = (B + C)*pow(10,n/2);

    printHeader('Z');
    D = parent_helper(port,a2,b2,size,pid);
    //calculate Z
    Z = D*pow(10,0);

    //calculate and print result
    int sum = X + Y + Z;
    printf("\n\n%d * %d == %d + %d + %d == %d\n",a, b, X, Y, Z, sum);
    return;
}

int parent_helper(int port[2][2], int a, int b, int size, pid_t pid){

    int W;

    //write a1 and b1 to pipe
    write(port[0][1],&a,size);
    write(port[0][1],&b,size);
    print_parent_send(pid,a,b);

    //read the result
    read(port[1][0],&W,size);
    print_parent_receive(pid,W);

    return W;
}

void child_functon(int port[2][2]){

    pid_t pid = getpid();
    int size = sizeof(int);
    //close unused ends of the pipe
    close(port[0][1]);
    close(port[1][0]);

    //call helper method to read from and write to the pipes
    child_helper(port,pid,size);

    child_helper(port,pid,size);

    child_helper(port,pid,size);

    child_helper(port,pid,size);

    exit(0);
}

void child_helper(int port[2][2], pid_t pid, int size){

    int a, b;
    //read a and b from pipe
    read(port[0][0],&a,size);
    read(port[0][0],&b,size);
    print_child_receive(pid,a,b);

    //multiply
    int A = a * b;

    //write A to pipe
    write(port[1][1],&A,size);
    print_child_send(pid,A);

}

int is_valid_integer(const char *str) {
    //Check if string is empty
    if(str == NULL || *str == '\0') {
        return 0;  
    }
    
    // Check for optional sign
    if(*str == '-' || *str == '+') {
        str++;
    }
    
    // Must have at least one digit
    if(*str == '\0') {
        return 0;
    }
    
    // Check all remaining characters are digits
    while(*str) {
        if(!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    
    return 1;
}

int int_length(int num) {
    // Special case for 0
    if (num == 0) return 1; 

    // Handle negative numbers
    if (num < 0) num = -num; 

    int length = 0;
    while (num > 0) {
        // Remove last digit
        num /= 10; 
        length++;
    }
    return length;
}


void printHeader(char letter){
    //print method
    printf("\n\n###\n");
    printf("# Calculating %c\n",letter);
    printf("###\n");
    return;
}

void print_parent_send(pid_t pid, int a, int b){
    //print method
    printf("Parent (PID %d): Sending %d to child\n",pid,a);
    printf("Parent (PID %d): Sending %d to child\n",pid,b);
    return;
}

void print_parent_receive(pid_t pid, int result){
    //print method
    printf("Parent (PID %d): Received %d from child \n",pid,result);
    return;
}

void print_child_send(pid_t pid, int result){
    //print method
    printf("Child (PID %d): Sending %d to parent\n",pid,result);
    return;
}

void print_child_receive(pid_t pid, int a, int b){
    //print method
    printf("Child (PID %d): Received %d from parent\n",pid,a);
    printf("Child (PID %d): Received %d from parent\n",pid,b);
    return;
}