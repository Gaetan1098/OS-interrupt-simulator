//Authors: Gaetan Fodjo 101273973 and Shifat Ghazi 101265285
//SYSC4001 Assignement 1
//Date: October 4th 2024

#include <stdlib.h>
#include <string.h>
#include "interrupts.h" 

int readTraceFile(const char *filename, struct Event *traceEvents){
    FILE *traceFile = fopen(filename, "r");
    if(!traceFile) {
        printf("Error opening file: %s\n", filename);
        return -1;
    }

    int i = 0;
    char line[256];
    while (fgets(line, sizeof(line), traceFile)){
        sscanf(line, "%[^,], %d", traceEvents[i].type, &traceEvents[i].duration); //Read & store  type and duration
        i++;
    }

    fclose(traceFile); 
    return i; //the number of events read
}

void readVectorTable(const char *filename, struct VectorTableEntry *vectorTable) {
    FILE *vectorFile = fopen(filename, "r");
    if(!vectorFile) {
        printf("Error opening vector table file: %s\n", filename);
        return;
    }

    int i = 0;
    char line[256];
    while (fgets(line, sizeof(line), vectorFile))
    {
        sscanf(line, "%s", vectorTable[i]. memory_address); //Read memory address for each ISR
        vectorTable[i].interrupt_number = i; //Interrupt number is the index in the vector table
        i++;
    }

    fclose(vectorFile); 
}

char *getTraceFileName(const char *shellScript){
    FILE *shellFile = fopen(shellScript, "r");
    if(!shellFile) {
        printf("Error opening file: %s\n", shellScript);
        return NULL;
    }

    static char traceFileName[100];
    char line[256];

    while (fgets(line, sizeof(line), shellFile)){
        if (strstr(line, "./sim") != NULL)
        {
            sscanf(line, "./sim %s", traceFileName); //Read & store trace file
            break;
        }
        
    }

    fclose(shellFile); 
    return traceFileName;
}

void simulateISR(FILE *logFile, int *currentTime, int isrNumber, int duration, struct VectorTableEntry *vectorTable){
    //log the interrupt handling steps
    fprintf(logFile, "%d, 1, switch to kernel mode\n", *currentTime);
    (*currentTime) += 1;

    fprintf(logFile, "%d, 3, context saved\n", *currentTime);
    (*currentTime) += 3;

    fprintf(logFile, "%d, 1, find vector %d in memory position 0x%04X\n", *currentTime , isrNumber, isrNumber*2);
    (*currentTime) += 1;

    fprintf(logFile, "%d, 1, load address %s into the PC\n", *currentTime, vectorTable[isrNumber].memory_address);
    (*currentTime) += 1;
    
    //Split the duration for SYSCALL execution steps
    int firstStep = duration / 2;
    int secondStep = duration / 3;
    int remaining = duration - (firstStep + secondStep);

    fprintf(logFile, "%d, %d, SYSCALL: run the ISR\n", *currentTime, firstStep); //First part of ISR execution
    (*currentTime) += firstStep;

    fprintf(logFile, "%d, %d, transfer data\n", *currentTime, secondStep); //Data transfer
    (*currentTime) += secondStep;


    fprintf(logFile, "%d, %d, check for errors\n", *currentTime, remaining); //Error checker
    (*currentTime) += remaining;

    fprintf(logFile, "%d, 1, IRET\n", *currentTime); //return from the interrupt
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

    fprintf(logFile, "%d, %d, END_IO\n", *currentTime, duration); //Log the END_IO Event
    (*currentTime) += duration;

    fprintf(logFile, "%d, 1, IRET\n", *currentTime); //return from the intterupt
    (*currentTime) += 1;
}

int runShellScript(const char *shellScript){
    int result = system(shellScript);
    if (result == -1)
    {
        printf("Error executing shell script: %s\n", shellScript);
        return -1;
    }

    return 1;
}

void runSimulation(struct Event *traceEvents, int eventCount, struct VectorTableEntry *vectorTable, const char *outputFilename){
    FILE *logFile = fopen(outputFilename, "w");
    if(!logFile){
        printf("Error opening log file: %s\n", outputFilename);
        return;
    }

    int currentTime = 0;

    //Loop through all the events in the trace
    for(int i = 0; i < eventCount; i++){
        if (strcmp(traceEvents[i].type, "CPU") == 0){
            fprintf(logFile, "%d, %d, CPU execution\n", currentTime, traceEvents[i].duration);
            currentTime += traceEvents[i].duration;
        }

        //Handle SYSCALL
        else if (strncmp(traceEvents[i].type, "SYSCALL", 7) == 0){
            //Extract ISR number from SYSCALL event in file and convert it into a integer
            int isrNumber = atoi(traceEvents[i].type + 7); 
            simulateISR(logFile, &currentTime, isrNumber, traceEvents[i].duration, vectorTable);
        }

        //Handle END_IO
        else if (strncmp(traceEvents[i].type , "END_IO", 6) == 0){
            //Extract ISR number from END_IO event in file and convert it into a integer
            int isrNumber = atoi(traceEvents[i].type + 6); 
            simulateEndIO(logFile,&currentTime, isrNumber, traceEvents[i].duration, vectorTable);
        }
    }

    fclose(logFile);
}

//main code with shell call arguments that calls trace files and outputs to execution files
int main(int argc, char *argv[]) {
    if (argc != 2){
        printf("Usage: %s <shell_file>\n", argv[0]);
        return -1;
    }

    char *traceFile = getTraceFileName(argv[1]);

    if(traceFile == NULL)
    {
        return -1; //error extracting the trace file name
    }

    if(runShellScript(argv[1]) != 1){
        return -1; //error running the shell script
    }

    struct Event traceEvents[MAX_EVENTS]; //to hold the traced events
    struct VectorTableEntry vectorTable[VECTOR_TABLE_SIZE]; //for the vector table

    readVectorTable("../additionalFiles/vector_table.txt", vectorTable);

    int eventCount = readTraceFile(traceFile, traceEvents);
    if (eventCount < 0) {
        return -1; //Error
    }

    //Logging and/or Creating the correct execution file
    if (strcmp(traceFile, "trace1.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "execution1.txt");
    } 
    
    else if (strcmp(traceFile, "trace2.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "execution2.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace3.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution3.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace4.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution4.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace5.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution5.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace6.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution6.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace7.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution7.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace8.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution8.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace9.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution9.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace10.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution10.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace11.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution11.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace12.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution12.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace13.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution13.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace14.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution14.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace15.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution15.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace16.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution16.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace17.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution17.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace18.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution18.txt");
    }

    else if (strcmp(traceFile, "../otherTests/trace19.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution19.txt");
    }
    else if (strcmp(traceFile, "../otherTests/trace20.txt") == 0) {
        runSimulation(traceEvents, eventCount, vectorTable, "../otherTests/execution20.txt");
    }

    printf("Simulation complete. Output written to execution file.\n");
    return 0;
}
