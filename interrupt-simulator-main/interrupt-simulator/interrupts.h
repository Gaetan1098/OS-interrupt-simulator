#ifndef INTERRUPTS_H

// Authors: Gaetan Fodjo 101273973 and Shifat Ghazi 101265285
// SYSC4001 Assignement 1
// Date: October 4th 2024

#define INTERRUPTS_H
#include <stdio.h>


//Maximum number of events and vector table size
#define MAX_EVENTS 250
#define VECTOR_TABLE_SIZE 25

//Structs to represent a single event in the trace
struct Event {
    char type[10]; // Event type such as CPU, SYSCALL, END_IO
    int duration; //duration in milliseconds
};

struct VectorTableEntry {
    int interrupt_number; // The number should match the SYSCALL number
    char memory_address[10];
};

//Function to read and parse the trace files
int readTracefile(const char*filename, struct Event *trace);

//Function to read and parse the vector table file
void readVectoreTable(const char *filename, struct VectorTableEntry *vectorTable);

//Function to simulate an ISR and log the process
void simulateISR(FILE *logFile, int *currentTime, int isrNumber, int duration, struct VectorTableEntry *vectorTable);

//Function to simulate an END_IO event
void simulateEndIO(FILE *logFile, int *currentTime, int isrNumber, int duration, struct VectorTableEntry *vectorTable);

//Function to run the simulation and log events into the output file
void runSimulation(struct Event *trace, int eventCount, struct VectorTableEntry *vectorTable, const char *outputFilename);

//Function to run the shell script
int runShellScript(const char *shellScript);

//Function to return the trace file name from the shell script
char *getTraceFileName(const char *shellScript);


#endif
