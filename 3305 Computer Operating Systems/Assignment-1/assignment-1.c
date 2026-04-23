#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>


/* 
Some of the code and ideas for the this function (is_valid_integer) was 
sourced from the following website:
https://www.codestudy.net/blog/how-to-check-if-a-string-is-a-number/#5-step-by-step-example-a-robust-numeric-string-checker
*/

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

void print_result(int count,long long int sum,int min, int max){
    //get process id of current process and parent
    pid_t pid = getpid();
    pid_t ppid = getppid();
    printf("pid: %d, ppid:%d - Count and sum of prime numbers between %d and %d are %d and %lld\n",pid,ppid,min,max,count,sum);
    return;
}

void calculate_primes(int min, int max){
    bool is_prime = true;
    long long int sum = 0;
    int count = 0;
    //check each number between min and max to see if it is prime
    for(long long int j = min; j<=max;j++){
        if(j<2){
            continue;
        }
        else{
            is_prime = true;
        for (int i = 2; i <=(j/2);i++){
            //when j%i==0, i divides j thus j is not prime
            if (j % i == 0){
                //printf("%d %d %d \n",j,i,(j%i));
                is_prime = false;
                break;
            }
        }
        if(is_prime){
            //j is prime add sum, and increment count
            count++;
            sum += j;
            //printf("%d is prime\n",j);
        }
        }
    }
    print_result(count,sum,min,max);
}



void serial_mode(int *quartiles){
    printf("Process id: %d\n",getpid());
    //call function calculate_primes for each quartile range
    for(int i = 0;i<4;i++){
        calculate_primes(quartiles[i],quartiles[i+1]);
    }
}

void parallel_mode(int *quartiles){
    printf("Process id: %d\n",getpid());
    pid_t pid;
    for(int i = 0;i<4;i++){
        //create child process
        pid = fork();
        //handle errors
        if(pid<0){
            perror("fork error\n");
            exit(0);
        }
        //child process runs computations on their respective quartiles
        if(pid==0){
            //printf("Child %d running computations\n",i);
            calculate_primes(quartiles[i],quartiles[i+1]);
            //break so we don't cause an infinite generation of childern processes
            break;
        }
        
    }
    //parent waits for all childern to finish
    if(pid>0){ 
        for(int i = 0;i<4;i++){
            wait(NULL);
        }
    }
}

int main(int argc, char *argv[]){
    //check number of arguments passed
    if (argc != 4){
        printf("Not enough arguments, exiting.\n");
        return 1;
    }

    //check validity of all arguments
    if(!is_valid_integer(argv[1]) || !is_valid_integer(argv[2]) ||!is_valid_integer(argv[3])){
        printf("Invalid integer entered, exiting.\n");
        return 1;
    }

    //store arguments
    int mode = atoi(argv[1]);
    int min = atoi(argv[2]);
    int max = atoi(argv[3]);
    //check if min > max
    if(min>=max){
        printf("The min is greater or equal to the max, exiting.\n");
        return 1;
    }
    if(min < 0 || max < 0){
        printf("Enter only positive numbers as the min and max, negative numbers are not prime.\n");
        return 1;
    }

    //calculate the quartiles
    int quartiles[5] = {min, max*1/4,max*2/4, max*3/4,max};

    if(mode == 0){
        serial_mode(quartiles);
    }
    else{
        parallel_mode(quartiles);
    }
    return 1;
}