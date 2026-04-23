# CPU Scheduling Simulator  

	This program simulate different types of CPU scheduling algorithms  

## Installation   

	Download the .tar zip and compile and run the code in a linux enviroment  

## Usage  

    To compile run the make command and the Makefile will generate an executable called asn4   
    The use case is as follows: ./asn4 mode infile    
    The mode should be a string with the following formats:  
        -f : For First Come First Served  
        -s : For Shortest Job First  
        -r : For Round Robin, for the Round RObin algorithm you must include a positive integer for the time quantum, so the use case will look like ./asn4 -r 4 infile  
    The input file must contain a list of processes and their burst time, and must be of the same structure/layout of the example input file  
        Where each line has the process number, which is also its arrival time, and its burst time of the following format   
            P0,3  