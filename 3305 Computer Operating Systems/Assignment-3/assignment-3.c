#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

// Structure to hold multiple arguments to be passed to the threads
typedef struct {
    int iteration;
    int thread_num;
    int (*board)[9];
    bool *is_valid;
} ThreadArgs;

#define MAX_NUMBER 27 

//function declarations
void* checksubgrid(void* arg);
void* checkrow(void* arg);
void* checkcol(void* arg);


int main(int argc, char *argv[]){

    //initiaization of variables
    int thread_counter = 0;
    int board[9][9];
    bool is_valid[MAX_NUMBER];
    ThreadArgs thread_args[MAX_NUMBER];
    pthread_t threads[MAX_NUMBER];


    //set all positions to ture
    for(int i = 0; i<MAX_NUMBER;i++){
        is_valid[i] = true;
    }

    //check for the correct amount of arguments was passed
    if (argc != 2){
        printf("Incorrect amount of arguments, exiting.\n");
        return 1;
    }

    //try to open file, exit if there is an error
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        return 1;
    }
    
    //read the exact number of integers needed for the board
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j ++){
            if (fscanf(file, "%d", &board[i][j]) != 1){
                printf("Either file finshed early(incorrect format of file) or there was an error reading the file\n");
                return 1;        // file finished or there was an error
            }
        }
    }
    //close file, since we are done with it
    fclose(file);

    //check all the subgrids
    for(int i = 0; i < 9;i++){
        thread_args[thread_counter].iteration = i;
        thread_args[thread_counter].thread_num = thread_counter;
        thread_args[thread_counter].board = board;       
        thread_args[thread_counter].is_valid = &is_valid[thread_counter]; 
        pthread_create(&threads[thread_counter], NULL, checksubgrid, &thread_args[thread_counter]);
        thread_counter++;
    }


    //check all the rows
    for(int i = 0; i < 9;i++){
        thread_args[thread_counter].iteration = i;
        thread_args[thread_counter].thread_num = thread_counter;
        thread_args[thread_counter].board = board;       
        thread_args[thread_counter].is_valid = &is_valid[thread_counter]; 
        pthread_create(&threads[thread_counter], NULL, checkrow, &thread_args[thread_counter]);
        thread_counter++;
    }

    //check all the columns
    for(int i = 0; i < 9;i++){
        thread_args[thread_counter].iteration = i;
        thread_args[thread_counter].thread_num = thread_counter;
        thread_args[thread_counter].board = board;       
        thread_args[thread_counter].is_valid = &is_valid[thread_counter]; 
        pthread_create(&threads[thread_counter], NULL, checkcol, &thread_args[thread_counter]);
        thread_counter++;
    }

    //join all the threads
    for(int i = 0; i < MAX_NUMBER; i++){
        if(pthread_join(threads[i], NULL)) 
            printf("Error joining with thread %d\n",i);
    }

    //check the final result
    bool valid_board_check = true;
    for(int i = 0; i< MAX_NUMBER;i++){
        if(!is_valid[i]){
            valid_board_check = false;
            break;
        }
    }

    //print the final result
    if(valid_board_check){
        printf("%s contains an valid solution \n",argv[1]);
    }else{
        printf("%s contains an INVALID solution \n",argv[1]);
    }

    return 0;
}

//method to check a subgrid of the board
void* checksubgrid(void* arg){

    //make sure the parameter passed is of the correct type
    ThreadArgs* args = (ThreadArgs*)arg;

    //get variables from the struct
    int itr = args->iteration;
    int thread_num = args->thread_num;


    //calculate the postion of the subarry to check
    int start_i = (itr/3)*3;
    int max_i = start_i + 2;
    int start_j = (itr%3)*3; 
    int max_j = start_j + 2;
    
    //initialize array to keep track of numbers we have seen
    bool check[10] = {false, false, false, false, false, false, false, false, false,false};
    bool valid = true;

    //check the subarray
    for(int i = start_i; i <= max_i; i++){
        for(int j = start_j ; j <= max_j; j++){
            int num = args->board[i][j];
            //if the number is out of the range, or we have seen the number before the subarray is invalid
            if (num < 1 || num > 9 || check[num]) {
                *args->is_valid = false;
                valid = false;
                break;
            }
            check[num] = true;

        }
        if(!valid) break;
    }
    if(valid) printf("Thread #  %d (subgrid %d) is valid\n",thread_num+1,itr+1);
    else printf("Thread #  %d (subgrid %d) is INVALID\n",thread_num+1,itr+1);

    return NULL;
}
void* checkrow(void* arg){

    //make sure the parameter passed is of the correct type
    ThreadArgs* args = (ThreadArgs*)arg;
    //get variables from the struct
    int itr = args->iteration;
    int thread_num = args->thread_num;


    //calculate the postion of the row to check
    int start_i = itr;
    int start_j = 0; 
    int max_j = 8;

    //initialize array to keep track of numbers we have seen
    bool check[10] = {false, false, false, false, false, false, false, false, false, false};
    bool valid = true;

    //check the row
    for(int j = start_j; j <= max_j; j++){
            int num = args->board[start_i][j];
            //if the number is out of the range, or we have seen the number before the row is invalid
            if (num < 1 || num > 9 || check[num]) {
                *args->is_valid = false;
                valid = false;
                break;
            }
            check[num] = true;
    }
    if(valid) printf("Thread #  %d (row %d) is valid\n",thread_num+1,itr+1);
    else printf("Thread #  %d (row %d) is INVALID\n",thread_num+1,itr+1);

    return NULL;
}
void* checkcol(void* arg){
    
    //make sure the parameter passed is of the correct type
    ThreadArgs* args = (ThreadArgs*)arg;
    //get variables from the struct
    int itr = args->iteration;
    int thread_num = args->thread_num;

    //calculate the postion of the row to check
    int start_i = 0;
    int max_i = 8;
    int start_j = itr;

    //initialize array to keep track of numbers we have seen
    bool check[10] = {false, false, false, false, false, false, false, false, false, false};
    bool valid = true;

    //check the column
    for(int i = start_i; i <= max_i; i++){
            int num = args->board[i][start_j];
            //if the number is out of the range, or we have seen the number before the column is invalid
            if (num < 1 || num > 9 || check[num]) {
                *args->is_valid = false;
                valid = false;
                break;
            }
            check[num] = true;
    }
    if(valid) printf("Thread #  %d (column %d) is valid\n",thread_num+1,itr+1);
    else printf("Thread #  %d (column %d) is INVALID\n",thread_num+1,itr+1);
    return NULL;
}