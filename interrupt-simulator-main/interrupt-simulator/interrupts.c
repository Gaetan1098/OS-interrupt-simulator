// Authors: Gaetan Fodjo 101273973 and Shifat Ghazi 101265285
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

    fprintf(logFile, "%d, 1, switch to kernel mode\n", *currentTime);
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

char *getTraceFileName(const char *shellScript){
    FILE *file = fopen(shellScript, "r");
    if(!file) {
        printf("Error opening file: %s\n", shellScript);
        return NULL;
    }

    static char traceFile[100];
    char line[256];

    while (fgets(line, sizeof(line), file)){
        if (strstr(line, "./sim") != NULL)
        {
            sscanf(line, "./sim %s", traceFile); //Read & store trace file
            break;
        }
        
    }

    fclose(file); 
    return traceFile;
}

int runShellScript(const char *shellScript){
    int result = system(shellScript);
    if (result != 0)
    {
        printf("Error executing shell script: %s\n", shellScript);
        return -1;
    }

    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2){
        printf("Usage: %s <shell_file>\n", argv[0]);
        return -1;
    }

    char *traceFile = getTraceFileName(argv[1]);

    if(traceFile == NULL)
    {
        return -1;// error extracting the trace file name
    }

    if(runShellScript(argv[1]) != 1){
        return -1; // error running the shell script
    }

    struct Event trace[MAX_EVENTS]; // to hold the traced events
    struct VectorTableEntry vectorTable[VECTOR_TABLE_SIZE]; // for the vector table

    readVectorTable("../additionalFiles/vector_table.txt", vectorTable);

    int eventCount = readTraceFile(traceFile, trace);
    if (eventCount < 0) {
        return -1; // Error
    }

    
    if (strcmp(traceFile, "trace1.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution1.txt");
    } 
    
    else if (strcmp(traceFile, "trace2.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "execution2.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace3.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution3.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace4.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution4.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace5.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution5.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace6.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution6.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace7.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution7.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace8.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution8.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace9.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution9.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace10.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution10.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace11.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution11.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace12.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution12.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace13.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution13.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace14.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution14.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace15.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution15.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace16.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution16.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace17.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution17.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace18.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution18.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace19.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution19.txt");
    }
    else if (strcmp(traceFile, "../otherTests/trace20.txt") == 0) {
        runSimulation(trace, eventCount, vectorTable, "../otherTests/execution20.txt");
    }

    printf("Simulation complete. Output written to execution file.\n");
    return 0;
}
