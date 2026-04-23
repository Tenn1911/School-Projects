#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

//struct to store all the data for each process
typedef struct {
    int arrival_time;
    int burst_time;
    int wait_time;
    int turnaround_time;
} Process;

//function declarations
int is_valid_integer(const char *str);
void first_come_first_served(Process *data, int size);
void shortest_job_first(Process *data, int size);
int get_next_index_sjf(int curr_process_index, int curr_tick, int size, Process *data);
void round_robin(Process *data, int size, int time_quantum);
int find_next_rr(Process *data, int size, int curr_index, int curr_tick);
bool check_finished(Process *data, int size);
void update_processes(int curr_index, Process *data, int size, int curr_tick);
void print_tick();
void print_result();

int main(int argc, char *argv[]){

    //check that the correct number of arguments was given
    if (argc != 3 && argc != 4){
        printf("Incorrect amount of arguments, exiting.\n");
        return 1;
    }

    //initialize variables to hold the given arguments
    char* algorithm = argv[1];
    char* filename;
    int time_quantum;

    //check the type of algorithm to correctly take in the rest of the arguments
    //and see if we need to take in a time quantum
    if(strcmp(algorithm, "-f") == 0 || strcmp(algorithm, "-s") == 0){
        filename = argv[2];

    } 
    else if (strcmp(algorithm, "-r") == 0){
        //check that the time quantum is a valid integer
        if(!is_valid_integer(argv[2])){
            printf("Invalid integer entered for the time quantum, exiting.\n");
            return 1;
        }
        time_quantum = atoi(argv[2]); 
        //check that the time quantum is a positive number
        if(time_quantum <= 0){
            printf("Integer entered for the time quantum is not positive, exiting.\n");
            return 1;
        }
        filename = argv[3];

    }
    else {
        printf("String entered for the algorithm is not valid, exiting.\n");
        return 1;
    }

    //open the given file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    //count the number of lines in the file
    //since the format of the file doesn't change, each line in the file represents a process
    int count = 0;
    char buffer[20];
    while (fgets(buffer, sizeof(buffer), file)) {
        count++;
    }

    //initialize memory to store our processes data
    Process *data = malloc(count * sizeof(Process));

    //set the file pointer to the begining of the file, to read the data
    rewind(file);

    //read the data from the file
    for(int i = 0; i < count;i++){
        if(fscanf(file, "P%d,%d\n", &data[i].arrival_time, &data[i].burst_time) == 2){
            data[i].wait_time = 0;
            data[i].turnaround_time = 0;
        }
        else{
            printf("Error reading data from file, exiting\n");
            free(data);
            return 1;
        }
    }
    //close the file once we are done
    fclose(file);

    //compare against the type of algorithm given
    //to call the correct method to simulate the given algorithm
    if (strcmp(algorithm, "-f") == 0) {
        // first come first serve
        first_come_first_served(data, count);
    }
    else if (strcmp(algorithm, "-s") == 0) {
        // shortest job first
        shortest_job_first(data, count);
    }
    else if (strcmp(algorithm, "-r") == 0) {
        // round robin
        round_robin(data, count, time_quantum);
    }
    else {
        // algorithm invalid
        printf("Invalid algorithm: %s\n", algorithm);
    }
    
    //free the allocated memory once we are done simulating
    free(data);
    return 0;
}

//method to simulate the First Come First Served algorithm
void first_come_first_served(Process *data, int size){
    //initiailze the tick count and initial index
    int curr_tick = 0;
    int curr_process_index = 0;

    printf("First Come First Served\n");
    //when curr_process_index is greater of equal to the size of the array
    //we have finished simulating the work of all the processes
    while(curr_process_index < size){
        //if the current process has not arrived yet, simulate busy waiting
        if (data[curr_process_index].arrival_time > curr_tick) {
            update_processes(-1, data, size, curr_tick);
            curr_tick++;
            continue;
        }
        //display the current process with its burst, waiting, and turnaroundtimes
        print_tick(curr_tick,data[curr_process_index].arrival_time,data[curr_process_index].burst_time,data[curr_process_index].wait_time,data[curr_process_index].turnaround_time);
        
        //decrement the burst time to simulate the process working
        data[curr_process_index].burst_time -= 1;
        
        //increment wait time for all processes except curr that have arrived and waiting
        //increment turn around time for all
        update_processes(curr_process_index,data,size,curr_tick);

        //update the current tick, since we finished all operations at the previous tick
        curr_tick++;

        //check if current process needs to be switched
        if(data[curr_process_index].burst_time == 0){
            curr_process_index++;
        }
    }
    //display the results
    print_result(data,size);
}


//method to simulate the Shortest Job First algorithm
void shortest_job_first(Process *data, int size){
    //initiailze the tick count and initial index
    int curr_tick = 0;
    int curr_process_index = 0;

    printf("Shortest Job First\n");
    while(true){
        
        //get the next process with the shortest burst time
        //and set the current index to the index of that process to simulate the preemptive part of the algorithm 
        curr_process_index = get_next_index_sjf(curr_process_index,curr_tick,size,data);
        if(curr_process_index == -1){
            //if there is no more work left we are done
            if (check_finished(data, size)){
                break;
            }
            //otherwise there are processes that have not arrived yet, so we simulate busy waiting
            update_processes(-1, data, size, curr_tick);
            curr_tick++; 
            continue;
        }
        //display the current process with its burst, waiting, and turnaroundtimes
        print_tick(curr_tick,data[curr_process_index].arrival_time,data[curr_process_index].burst_time,data[curr_process_index].wait_time,data[curr_process_index].turnaround_time);
        
        //decrement the burst time to simulate the process working
        data[curr_process_index].burst_time -= 1;

        //update the wait and turnaround time
        update_processes(curr_process_index,data,size,curr_tick);

        //update the current tick, since we finished all operations at the previous tick
        curr_tick++;

    }
    //display the results
    print_result(data,size);
}

//helper method for the sjf algorithm to get the next index
int get_next_index_sjf(int curr_process_index, int curr_tick, int size, Process *data){
    //initialize variables to hold the shortest burst time and next index
    int shortest_burst_time = INT_MAX;
    int next_index = -1;
    //loop over the array of processes to find the one with the shortest burst time, that has arrived, and still has burst time left
    for(int i = 0; i< size; i++){
        if(0 < data[i].burst_time 
            && data[i].burst_time < shortest_burst_time 
            && data[i].arrival_time <= curr_tick){
            shortest_burst_time = data[i].burst_time;
            next_index = i;
        }
    }
    //return -1 if all processes have finished
    if(shortest_burst_time == INT_MAX || next_index == -1){
        return -1;
    }
    else{
        //return the index of the next process to run
        return next_index;
    }
}

//method to simulate the Round Robin with Quantum algorithm
void round_robin(Process *data, int size, int time_quantum){
    //initiailze the tick count, initial index, and a temporary quantum
    int curr_tick = 0;
    int curr_process_index = 0;
    int temp_time_quantum = time_quantum;

    printf("Round Robin with Quantum %d\n",time_quantum);
    //every iteratation we check to see if we have finsihed all the work
    while(!check_finished(data, size)){
        
        //if the temporary quatum has ended, we get the next processes index to become the active process
        //and renew the quantum
        if(temp_time_quantum == 0){
            curr_process_index = find_next_rr(data,size,curr_process_index,curr_tick);
            temp_time_quantum = time_quantum;
        }
        
        //check if the current process has finished its work or if the current process has arrived
        //if it has find the index of the next process to become the active procss
        if(data[curr_process_index].burst_time == 0 || data[curr_process_index].arrival_time > curr_tick){
            int next = find_next_rr(data,size,curr_process_index,curr_tick);
            if(next == -1){
                //simulate busy wait
                update_processes(-1, data, size, curr_tick); 
                curr_tick++;
                continue;
            }
            curr_process_index = next;
            temp_time_quantum = time_quantum;
        }        
        
        //display the current process with its burst, waiting, and turnaroundtimes
        print_tick(curr_tick,data[curr_process_index].arrival_time,data[curr_process_index].burst_time,data[curr_process_index].wait_time,data[curr_process_index].turnaround_time);
        
        //decrement the burst time to simulate the process working
        data[curr_process_index].burst_time -= 1;

        //update the wait and turnaround time
        update_processes(curr_process_index,data,size,curr_tick);

        //update the current tick and quantum, since we finished all operations at the previous tick
        curr_tick++;
        temp_time_quantum--;

    }
    //display the results
    print_result(data,size);
}

//helper method to find the next index for the rr algorithm
int find_next_rr(Process *data, int size, int curr_index, int curr_tick){
    //initialize a variable to hold the next index
    int next = curr_index;
    //loop over the array of processes once
    for(int i = 0; i <size; i++){
        //start from the current index and iterate within the array
        next = (next +1) % size;
        //check if the process has arrived and has work
        if (data[next].burst_time > 0 && data[next].arrival_time <= curr_tick) {
            return next;
        }
    }
    // return -1, when the processes that stil have burst time have not yet arrived
    return -1;
}

//helper method for the rr algorithm to check if all the processes have finished their work
bool check_finished(Process *data, int size){
    //loop over the array of processes and check if any of them have burst time left
    for(int i = 0; i < size; i++){
        if(data[i].burst_time > 0){
            //if they do return false, and keep simulating work
            return false;
        }
    }
    //return true if all the work is done
    return true;
}

//helper method to update the turnaround times and wait times for the processes
void update_processes(int curr_index, Process *data, int size, int curr_tick){
    //loop over all processes once
    for(int i = 0; i<size; i++){
        //only update the turnaround time for the current running processes
        if(curr_index == i){
            data[i].turnaround_time++;
        } 
        //for each process that has arrived and has burst time left
        //update both their wait time and turnaround time
        else{
            if(data[i].arrival_time <= curr_tick && data[i].burst_time > 0){
                data[i].wait_time++;
                data[i].turnaround_time++;
            }
        }
    }
}

//helper method to print the operations at the current tick
void print_tick(int tick_time, int process_num, int burst_left, int wait_time, int turnaround_time){
    printf("T%d : P%d - Burst left %d, Wait time %d, Turnaround time %d\n",
        tick_time, process_num, burst_left, wait_time, turnaround_time);
}

//helper method to print the final results of the algorithm
void print_result(Process *data, int size){
    //initialize variables to hold the sums 
    int wait_time_sum = 0;
    int turnaround_time_sum = 0;

    for(int i = 0; i < size; i++){
        //increment the sums
        wait_time_sum += data[i].wait_time;
        turnaround_time_sum += data[i].turnaround_time;

        //print each of the processes resulting data
        printf("P%d\n",data[i].arrival_time);
        printf("\tWaiting time:   %11d\n",data[i].wait_time);
        printf("\tTurnaround time:%11d\n\n",data[i].turnaround_time);
    }

    //calculate the averages
    float avg_wait_time, avg_turnaround_time;
    avg_wait_time = (float)wait_time_sum/(float)size;
    avg_turnaround_time = (float)turnaround_time_sum/(float)size;
    //print the result
    printf("Total average waiting time:     %.1f\n",avg_wait_time);
    printf("Total average turnaround time:  %.1f\n",avg_turnaround_time);
}

//mehtod to validate that a string is an integer
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
    //return 1 if the string is an integer
    return 1;
}