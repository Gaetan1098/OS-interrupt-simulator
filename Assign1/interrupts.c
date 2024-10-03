// Authors: Gaetan Fodjo 101273973 and Shifat Gazi
// SYSC4001 Assignement 1
// Date: October 4th 2024

#include <stdlib.h>
#include <string.h>
#include "interrupts.h" 

int readTraceFile(const char *filename, struct Event *trace){
    FILE *file = fopen(filename, "r");
    if(!file) {
        printf("Error opening file: %s\n", filename);
        return -1;
    }

    int i = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)){
        sscanf(line, "%[^,], %d", trace[i].type, &trace[i].duration); //Read & store  type and duration
        i++;
    }

    fclose(file); 
    return i; // the number of events read
}

void readVectorTable(const char *filename, struct VectorTableEntry *vectorTable) {
    FILE *file = fopen(filename, "r");
    if(!file) {
        printf("Error opening vector table file: %s\n", filename);
        return;
    }

    int i = 0;
    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        sscanf(line, "%s", vectorTable[i]. memory_address); // Read memory address for each ISR
        vectorTable[i].interrupt_number = i; // Interrupt number is the index in the vector table
        i++;
    }

    fclose(file); 
}

void simulateISR(FILE *logFile, int *currentTime, int isrNumber, int duration, struct VectorTableEntry *vectorTable){
    // log the interrupt handling steps
    fprintf(logFile, "%d, 1, switch to kernel mode\n", *currentTime);
    (*currentTime) += 1;

    fprintf(logFile, "%d, 3, context saved\n", *currentTime);
    (*currentTime) += 3;

    fprintf(logFile, "%d, 1, find vector %d in memory position 0x%04X\n", *currentTime , isrNumber, isrNumber*2);
    (*currentTime) += 1;

    fprintf(logFile, "%d, 1, load address %s into the PC\n", *currentTime, vectorTable[isrNumber].memory_address);
    (*currentTime) += 1;
    
    // Split the duration for SYSCALL execution steps
    int firstPhase = duration / 2;
    int secondPhase = duration / 3;
    int remaining = duration - (firstPhase + secondPhase);

    fprintf(logFile, "%d, %d, SYSCALL: run the ISR\n", *currentTime, firstPhase); // First part of ISR execution
    (*currentTime) += firstPhase;

    fprintf(logFile, "%d, %d, transfer data\n", *currentTime, secondPhase); // Data transfer
    (*currentTime) += secondPhase;


    fprintf(logFile, "%d, %d, check for errors\n", *currentTime, remaining); // Error checker
    (*currentTime) += remaining;

    fprintf(logFile, "%d, 1, IRET\n", *currentTime); // return from the interrupt
    (*currentTime) += 1;
}

void simulateEndIO(FILE *logFile, int *currentTime, int isrNUmber, int duration, struct VectorTableEntry *vectorTable){
    fprintf(logFile, "%d, 1, check priority of interrupt\n", *currentTime);
    (*currentTime) += 1;

    fprintf(logFile, "%d, 1 , check if masked\n", *currentTime);
    (*currentTime) += 1;

    fprintf(logFile, "%d, 1, switch to kernel moed\n", *currentTime);
    (*currentTime) += 1;

    fprintf(logFile, "%d, 3, context saved\n", *currentTime);
    (*currentTime) += 3;

    fprintf(logFile, "%d, 1, find vector %d in memory position %x04X\n", *currentTime, isrNUmber, isrNUmber*2);
    (*currentTime) += 1;

    fprintf(logFile, "%d, 1, load address %s into the PC\n", *currentTime, vectorTable[isrNUmber].memory_address);
    (*currentTime) += 1;

    fprintf(logFile, "%d, %d, END_IO\n", *currentTime, duration); // Log the END_IO Event
    (*currentTime) += duration;

    fprintf(logFile, "%d, 1, IRET\n", *currentTime); // return from the intterupt
    (*currentTime) += 1;
}

// main simulation function
void runSimulation(struct Event *trace, int eventCount, struct VectorTableEntry *vectorTable, const char *outputFilename){
    FILE *logFile = fopen(outputFilename, "w");
    if(!logFile){
        printf("Error opening log file: %s\n", outputFilename);
        return;
    }

    int currentTime = 0;

    // Loop through all the events in the trace
    for(int i = 0; i < eventCount; i++){
        if (strcmp(trace[i].type, "CPU") == 0){
            fprintf(logFile, "%d, %d, CPU execution\n", currentTime, trace[i].duration);
            currentTime += trace[i].duration;
        }
        
        else if (strncmp(trace[i].type, "SYSCALL", 7) == 0){
            // Handle SYSCALL
            int isrNumber = atoi(trace[i].type + 7); // Extract ISR number from SYSCALL number
            simulateISR(logFile, &currentTime, isrNumber, trace[i].duration, vectorTable);
        }

        else if (strncmp(trace[i].type , "END_IO", 6) == 0){
            // Handle END_IO
            int isrNumber = atoi(trace[i].type + 6); // Extract the ISR number from the END_IO call
            simulateEndIO(logFile,&currentTime, isrNumber, trace[i].duration, vectorTable);
        }
    }

    fclose(logFile);
}

int main(int argc, char *argv[]) {
    if (argc != 2){
        printf("Usage: %s <trace_file>\n", argv[0]);
        return -1;
    }

    struct Event trace[MAX_EVENTS]; // to hold the traced events
    struct VectorTableEntry vectorTable[VECTOR_TABLE_SIZE]; // for the vector table

    readVectorTable("vector_table.txt", vectorTable);

    int eventCount = readTraceFile(argv[1], trace);
    if (eventCount < 0) {
        return -1; // Error
    }

    
    if (strcmp(argv[1], "trace1.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution1.txt");
    } 
    
    else if (strcmp(argv[1], "trace2.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution2.txt");
    }

    else if (strcmp(argv[1], "trace3.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution3.txt");
    }

    else if (strcmp(argv[1], "trace4.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution4.txt");
    }

    else if (strcmp(argv[1], "trace5.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution5.txt");
    }

    else if (strcmp(argv[1], "trace6.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution6.txt");
    }

    else if (strcmp(argv[1], "trace7.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution7.txt");
    }

    else if (strcmp(argv[1], "trace8.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution8.txt");
    }

    else if (strcmp(argv[1], "trace9.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution9.txt");
    }

    else if (strcmp(argv[1], "trace10.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution10.txt");
    }

    else if (strcmp(argv[1], "trace11.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution11.txt");
    }

    else if (strcmp(argv[1], "trace12.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution12.txt");
    }

    else if (strcmp(argv[1], "trace13.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution13.txt");
    }

    else if (strcmp(argv[1], "trace14.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution14.txt");
    }

    else if (strcmp(argv[1], "trace15.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution15.txt");
    }

    else if (strcmp(argv[1], "trace16.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution16.txt");
    }

    else if (strcmp(argv[1], "trace17.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution17.txt");
    }

    else if (strcmp(argv[1], "trace18.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution18.txt");
    }

    else if (strcmp(argv[1], "trace19.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution19.txt");
    }
    else if (strcmp(argv[1], "trace20.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution20.txt");
    }

    printf("Simulation complete. Output written to execution file.\n");
    return 0;
}
