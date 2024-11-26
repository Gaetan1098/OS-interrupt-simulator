#ifndef INTERRUPTS_H

//Authors: Gaetan Fodjo and Shifat Ghazi

#define INTERRUPTS_H
#include <stdio.h>


//Maximum number of events and vector table size
#define MAX_EVENTS 250
#define VECTOR_TABLE_SIZE 25

//Structs to represent a single event in the trace
struct Event {
    char type[10]; 
    int event_number;     
    int duration;  
    char file_name[10]; 
};

struct VectorTableEntry {
    int interrupt_number; //The number should match the SYSCALL number
    char memory_address[10]; //Vector stored in the row
};

struct Memory_Partition{
    int partition_number;
    int size;
    char code[10];  
};

struct PCB{
    int pid;
    char program_name[32];  
    int memory_partition; // Index of the memory partition allocated.
    int size;
} ;

struct ExternalFiles{
    char program_name[20];
    int program_size;
};


//Function to read and parse the trace files
int readTracefile(const char*filename, struct Event *trace);

//Function to read and parse the vector table file
void readVectoreTable(const char *filename, struct VectorTableEntry *vectorTable);

//Function to return the trace file name from the shell script
char *getTraceFileName(const char *shellScript);

//Function to simulate an ISR and log the process
void simulateISR(FILE *logFile, int *currentTime, int isrNumber, int duration, struct VectorTableEntry *vectorTable);

//Function to simulate an END_IO event
void simulateEndIO(FILE *logFile, int *currentTime, int isrNumber, int duration, struct VectorTableEntry *vectorTable);

//Function to simulate an Fork event
void simulateFork(FILE *logFile, int *currentTime, int isrNumber, int duration, struct VectorTableEntry *vectorTable);

//Function to simulate an Exec event
void simulateExec(FILE *logFile, int *currentTime, int isrNumber, int duration, struct VectorTableEntry *vectorTable, const char *filename);

void simulateOutput(FILE *logFile);

//Function to run the shell script
int runShellScript(const char *shellScript);

//Function to run the simulation and log events into the output file
void runSimulation(struct Event *trace, int eventCount, struct VectorTableEntry *vectorTable, FILE *logFile);



#endif
