/*
This program simulates two trains picking up and dropping of passengers at different stations
run 'make' to create an executable, and ./asn5 to run the program
@author Tennyson Hung 
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>

//Define the size of the array to hold the stations
#define ARRAY_SIZE 5

//Define the capacity of train 0
#define TRAIN_0_CAPACITY 100

//Define the capacity of train 1
#define TRAIN_1_CAPACITY 50

//Custom type to hold the data for each station
typedef struct {
    //the current number of passengers at the station
    int passengers;
    //the maximum number of passengers the station can hold
    int limit;
    //the lock the control the data of the station
    pthread_mutex_t lock;
} Station;

//Array declaration for the 5 stations
Station stations[ARRAY_SIZE];

//Array declaration for the threads
pthread_t tid[2];

//STDOUT lock declaration
pthread_mutex_t stdout_lock;

//Function declarations
void initialize_stations();
void destory_station_locks();
void* simulate_trains(void* arg);
int pick_up_passengers(int train_number,int current_train_capacity, int train_max_capacity);
int drop_off_passengers(int station_num, int train_number, int current_train_capacity, int train_max_capacity);
bool validate_current_station(int station_num);
void print_error(int thread_num, const char* msg);

//The main function
int main(){
    //initialize the stations 0 - 4 
    initialize_stations();
    //initialize the stdout lock
    pthread_mutex_init(&stdout_lock, NULL);

    //create and run threads 
    int train_ids[2];
    for (int i = 0; i <2; i ++){
        train_ids[i] = i;
         pthread_create(&tid[i], NULL, simulate_trains, &train_ids[i]);
    }

    //join the threads back together
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    //destory station locks and STDOUT lock
    pthread_mutex_destroy(&stdout_lock);
    destory_station_locks();

    //check that stations end with correct number of passengers
    /*
    for(int i = 0; i < ARRAY_SIZE; i++){
        printf("Station %d: %d passengers\n",i,stations[i].passengers);
    }
    */

    return 0;
}

//Initialize the station arrays elements
void initialize_stations(){

    //initialize station 0's passengers and lock
    stations[0].passengers = 500;
    pthread_mutex_init(&stations[0].lock, NULL);

    //initialize station 1-4's passengers and locks
    for (int i = 1; i < ARRAY_SIZE; i++) {
        stations[i].passengers = 0;
        pthread_mutex_init(&stations[i].lock, NULL);
    }

    //set the limit for each station
    //station 0 has no limit since pasengers are only picked up
    stations[0].limit = 0;
    stations[1].limit = 50;
    stations[2].limit = 100;
    stations[3].limit = 250;
    stations[4].limit = 100;

    return;
}

//Helper function to destory/deallocate all the locks of the station array
void destory_station_locks(){
    for (int i = 0; i < ARRAY_SIZE; i++) {
        pthread_mutex_destroy(&stations[i].lock);
    }
}

//Funcion the threads are called upon
void* simulate_trains(void* args){

    //get the train numner
    int train_num = *(int *)args;

    //find the max capacity of the train
    int max_capacity;
    if (train_num == 0){
        max_capacity = TRAIN_0_CAPACITY;
    }
    else if(train_num == 1){
        max_capacity = TRAIN_1_CAPACITY;
    }
    else{
        print_error(train_num, "Incorrect argument passed to thread\n");
        return NULL;
    }
    
    //initiaize the trains current capacity, current location, and next location
    int current_capacity = 0;
    int current_station = 0;
    int next_station = current_station;

    while(true){
        //lock the rescourses
        pthread_mutex_lock(&stdout_lock);
        pthread_mutex_lock(&stations[current_station].lock);

        //check the train is at a valid station
        if(!validate_current_station(current_station)){
            printf("Current station invalid, exiting thread %d.\n",train_num);
            pthread_exit(NULL);
        }

        printf("Train %d ENTERS Station %d\n", train_num, current_station);

        //when current station is 0, pick up passengers
        if(current_station == 0){
            current_capacity = pick_up_passengers(train_num, current_capacity, max_capacity);
            //if the train is not empty, go to the next station
            if(current_capacity != 0){
                next_station += 1;
            }
        }
        //otherwise we are dropping passengers of
        else{
            current_capacity = drop_off_passengers(current_station, train_num, current_capacity, max_capacity);
            if(current_capacity == 0){
                //train empty go back towards station 0
                next_station -= 1;
            }
            else{
                //train still has passengers to drop off, get the next station number
                next_station += 1;
            }
        }
        printf("Train %d LEAVES Station %d\n", train_num, current_station);

        //unlock the rescourses
        pthread_mutex_unlock(&stdout_lock);
        pthread_mutex_unlock(&stations[current_station].lock);

        //check if train is leaving station 0 with 0 pasengers
        if(current_station == 0 && next_station == 0 && current_capacity == 0){
            //if it is then the train is done its work
            break;
        }
        //simulate travel between stations
        sleep(1);

        //the train arrives at the next station
        current_station = next_station;
    }
    //end of the thread
    return NULL;
}

//Helper function to pick up passengers at a station
int pick_up_passengers(int train_number,int current_train_capacity, int train_max_capacity){

    //check how many passengers the station has left to pick up
    printf("    Station 0 has %d passengers left to pick up\n", stations[0].passengers);
    
    //get the number of passengers the train can hold
    printf("    Train %d is at Station 0 and has %d/%d passengers\n", train_number, current_train_capacity, train_max_capacity);

    //when station 0 has 0 passengers, do nothing
    if(stations[0].passengers == 0){
        printf("        <Nothing more to do>...\n");
    }
    //otherwise pick up as many passenger that can fit in the train
    else if (stations[0].passengers > 0){
        printf("        Loading passengers...\n");
        //get the number of passengers boarding
        int num_passengers_boarding = train_max_capacity - current_train_capacity;

        if(stations[0].passengers > num_passengers_boarding){
            //simulate picking up passengers
            sleep(num_passengers_boarding/10);

            current_train_capacity += num_passengers_boarding;
            stations[0].passengers -= num_passengers_boarding;
        }
        else{
            //simulate picking up passengers
            sleep(stations[0].passengers/10);

            current_train_capacity += stations[0].passengers;
            stations[0].passengers -= stations[0].passengers;
        }
    }
    //catch any errors
    else{
        printf("        Negative number of passengers at station 0, with train %d\n",train_number);
    }

    //print the number of passenger on the train
    printf("    Train %d is at Station 0 and has %d/%d passengers\n", train_number, current_train_capacity, train_max_capacity);
    
    //print the number of passengers waiting at the station
    printf("    Station 0 has %d passengers left to pick up\n", stations[0].passengers);

    //return the number of passenger on the train
    return current_train_capacity;
}

//Helper function to drop off passengers at a station
int drop_off_passengers(int station_num, int train_number, int current_train_capacity, int train_max_capacity){

    //check how many passengers the station has left to arrive
    int passengers_left_to_arrive = stations[station_num].limit - stations[station_num].passengers;
    printf("    Station %d has %d passengers left to arrive\n", station_num, passengers_left_to_arrive);

    //get the number of passengers the train can drop off
    printf("    Train %d is at Station %d and has %d/%d passengers\n", train_number, station_num, current_train_capacity, train_max_capacity);

    //the station is full do nothing
    if(passengers_left_to_arrive == 0){
        printf("        <Nothing more to do>...\n");
    }
    //the station still has capacity for more passengers
    else if(passengers_left_to_arrive > 0){
        printf("        Unloading passengers...\n");
        if(passengers_left_to_arrive > current_train_capacity){
            //simulate dropping off passengers
            sleep(current_train_capacity/10);
            stations[station_num].passengers += current_train_capacity;
            current_train_capacity -= current_train_capacity;
        }
        else{
            //simulate dropping off passengers
            sleep(passengers_left_to_arrive/10);
            stations[station_num].passengers += passengers_left_to_arrive;
            current_train_capacity -= passengers_left_to_arrive;
        }
    }
    //catch any errors
    else{
        printf("    Negative number of passengers at station %d, with train %d\n", station_num, train_number);
    }

    //print the number of passenger left on the train
    printf("    Train %d is at Station %d and has %d/%d passengers\n", train_number, station_num, current_train_capacity, train_max_capacity);
     
    //print the number of passengers the station is waitng for
    passengers_left_to_arrive = stations[station_num].limit - stations[station_num].passengers;
    printf("    Station %d has %d passengers left to arrive\n",station_num, passengers_left_to_arrive);

    //return the number of passenger on the train
    return current_train_capacity;
}

//Helper method to ensure the given station is a valid one
bool validate_current_station(int station_num){
    if(station_num >= 0 || station_num<=4){
        return true;
    }
    return false;
}

//Helper method to lock STDOUT and print any error messages
void print_error(int thread_num, const char* msg){

    pthread_mutex_lock(&stdout_lock);

    fprintf(stderr, "THREAD %d:\nERROR: %s\n",thread_num, msg); 

    pthread_mutex_unlock(&stdout_lock);
}