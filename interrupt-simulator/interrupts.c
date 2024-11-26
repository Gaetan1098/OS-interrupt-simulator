//Authors: Gaetan Fodjo 101263973  and Shifat Ghazi 101265285
//TO RUN CODE:
// gcc interrupts_101263973_101265285.c -o interrupts
// ./interrupts test_101263973_101265285.sh external_files_101263973_101265285.txt
// TEST FILE WAS SUBMITTED AS A .txt, CHANGE IT TO .sh
#include <stdlib.h>
#include <string.h>
#include "interrupts_101263973_101265285.h" 

struct PCB pcbTable[25];
int pcbElements = 0;
struct Memory_Partition partitions[6];

struct ExternalFiles programs[10];

int execCalls = 1;
int forkCalls = 0;

int currentTime = 0;

FILE *system_status;

int readTraceFile(const char *filename, struct Event *traceEvents) {
    FILE *file = fopen(filename, "r");
    // Error handling
    if (!file) {
        printf("Cannot open the needed file: %s\n", filename);
        return -1;
    }

    // Go over each line until no more lines
    int i = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "SYSCALL", 7) == 0) {
            sscanf(line, "%s %d, %d", traceEvents[i].type, &traceEvents[i].event_number, &traceEvents[i].duration);
            strcpy(traceEvents[i].file_name, "");
        } 
        else if (strncmp(line, "CPU", 3) == 0) {
            sscanf(line, "%[^,], %d", traceEvents[i].type, &traceEvents[i].duration);
            traceEvents[i].event_number = -1; 
            strcpy(traceEvents[i].file_name, "");
        } 
        else if (strncmp(line, "FORK", 4) == 0) {
            sscanf(line, "%[^,], %d", traceEvents[i].type, &traceEvents[i].duration);
            traceEvents[i].event_number = -1;  // FORK doesn't have an event number
            strcpy(traceEvents[i].file_name, "");
        } 
        else if (strncmp(line, "EXEC", 4) == 0) {
            sscanf(line, "%s %[^,], %d", traceEvents[i].type, traceEvents[i].file_name, &traceEvents[i].duration);
            traceEvents[i].event_number = -1; 
        } 
        else if (strncmp(line, "END_IO", 6) == 0) {
            sscanf(line, "%s %d, %d", traceEvents[i].type, &traceEvents[i].event_number, &traceEvents[i].duration);
            strcpy(traceEvents[i].file_name, "");
        }
        i++;
    }

    // Close the file and return the number of events
    fclose(file);
    return i;
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

char *getOutputFileName(const char *shellScript){
    FILE *shellFile = fopen(shellScript, "r");
    if(!shellFile) {
        printf("Error opening file: %s\n", shellScript);
        return NULL;
    }

    static char traceFileName[100];
    static char outputFileName[100];
    char line[256];

    while (fgets(line, sizeof(line), shellFile)){
        if (strstr(line, "./sim") != NULL)
        {
            sscanf(line, "./sim %s %s", traceFileName, outputFileName); //Read & store trace file
            break;
        }
        
    }

    fclose(shellFile); 
    return outputFileName;
}



int readExternalFiles(const char *filename, struct ExternalFiles *programs){
    FILE *externalFile = fopen(filename, "r");
    if(!externalFile) {
        printf("Error opening program file: %s\n", filename);
        return -1;
    }

    int i = 0;
    char line[256];
    while (fgets(line, sizeof(line), externalFile))
    {
        if (line[0] == '\n'){
            continue; // to skip empty lines
        }
        sscanf(line, "%[^,], %d", programs[i].program_name, &programs[i].program_size); //Read program name and program size
        i++;
    }

    fclose(externalFile); 
    return i; // return the number of programs loaded
}

void simulateISR(FILE *logFile, int *currentTime, int isrNumber, int duration, struct VectorTableEntry *vectorTable){
    //log the interrupt handling steps
    fprintf(logFile, "%d, 1, switch to kernel mode\n", *currentTime);
    (*currentTime) += 1;

    //generating random number from 1-3 
    int contextDuration = rand() % (3) + 1;
    fprintf(logFile, "%d, %d, context saved\n", *currentTime, contextDuration);
    (*currentTime) += contextDuration;

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

    fprintf(logFile, "%d, 1, check if masked\n", *currentTime);
    (*currentTime) += 1;

    fprintf(logFile, "%d, 1, switch to kernel mode\n", *currentTime);
    (*currentTime) += 1;

    int contextDuration = rand() % (3) + 1;
    fprintf(logFile, "%d, %d, context saved\n", *currentTime, contextDuration);
    (*currentTime) += contextDuration;

    fprintf(logFile, "%d, 1, find vector %d in memory position %x04X\n", *currentTime, isrNUmber, isrNUmber*2);
    (*currentTime) += 1;

    fprintf(logFile, "%d, 1, load address %s into the PC\n", *currentTime, vectorTable[isrNUmber].memory_address);
    (*currentTime) += 1;

    fprintf(logFile, "%d, %d, END_IO\n", *currentTime, duration); //Log the END_IO Event
    (*currentTime) += duration;

    fprintf(logFile, "%d, 1, IRET\n", *currentTime); //return from the intterupt
    (*currentTime) += 1;
}

void simulateFork(FILE *logFile, int *currentTime, int isrNumber, int duration, struct VectorTableEntry *vectorTable){
    forkCalls++;
     //log the interrupt handling steps
    fprintf(logFile, "%d, 1, switch to kernel mode\n", *currentTime);
    (*currentTime) += 1;

    //generating random number from 1-3 
    int contextDuration = rand() % (3) + 1;
    fprintf(logFile, "%d, %d, context saved\n", *currentTime, contextDuration);
    (*currentTime) += contextDuration;

    fprintf(logFile, "%d, 1, find vector %d in memory position 0x%04X\n", *currentTime , isrNumber, isrNumber*2);
    (*currentTime) += 1;

    fprintf(logFile, "%d, 1, load address %s into the PC\n", *currentTime, vectorTable[isrNumber].memory_address);
    (*currentTime) += 1;
    
    //Split the duration for FORK execution steps
    int firstStep = duration / 3;
    int remaining = duration - firstStep;

    int pcbIndex = pcbElements -1; // starting from the end of the pcb table

    while(pcbIndex >= 0 && strcmp(pcbTable[pcbIndex].program_name, "init")!= 0){
        pcbIndex--; // going to the front
    }

    if(pcbIndex >= 0){
        pcbTable[forkCalls].pid = pcbTable[pcbElements-1].pid + 1;
        pcbTable[forkCalls].memory_partition = pcbTable[pcbIndex].memory_partition;
        strcpy(pcbTable[forkCalls].program_name, pcbTable[pcbIndex].program_name);
        pcbTable[forkCalls].size = pcbTable[pcbIndex].size;
    }

    fprintf(logFile, "%d, %d, FORK: copy parent PCB to child PCB\n", *currentTime, firstStep); //First part of FORK execution
    (*currentTime) += firstStep;

    fprintf(logFile, "%d, %d, scheduler called\n", *currentTime, remaining);
    (*currentTime) += remaining;

    simulateOutput(system_status);

    fprintf(logFile, "%d, 1, IRET\n", *currentTime); //return from the interrupt
    (*currentTime) += 1;

    pcbElements++;


}

void simulateExec(FILE *logFile, int *currentTime, int isrNumber, int duration, struct VectorTableEntry *vectorTable, const char *filename){
     
    //log the interrupt handling steps
    fprintf(logFile, "%d, 1, switch to kernel mode\n", *currentTime);
    (*currentTime) += 1;

    //generating random number from 1-3 
    int contextDuration = rand() % (3) + 1;
    fprintf(logFile, "%d, %d, context saved\n", *currentTime, contextDuration);
    (*currentTime) += contextDuration;

    fprintf(logFile, "%d, 1, find vector %d in memory position 0x%04X\n", *currentTime , isrNumber, isrNumber*2);
    (*currentTime) += 1;

    fprintf(logFile, "%d, 1, load address %s into the PC\n", *currentTime, vectorTable[isrNumber].memory_address);
    (*currentTime) += 1;

    //Split the duration for EXEC execution steps
    int firstStep = duration / 5;
    int secondStep = duration / 4;
    int thirdStep = duration / 7;
    int fourthStep = duration / 4;
    int remaining = duration - (firstStep + secondStep + thirdStep + fourthStep); 

    fprintf(logFile, "%d, %d, EXEC: load %s of size %dmB\n", *currentTime, firstStep, filename, programs[execCalls-1].program_size); //First part of EXEC execution
    (*currentTime) += firstStep;

    int partition;
    for (int i = 0; i < 6; i++)
    {
       if (programs[execCalls-1].program_size == partitions[i].size) 
       {
            fprintf(logFile, "%d, %d, found partition %d with %dmB of space\n", *currentTime, secondStep, i+1, partitions[i].size); 
            (*currentTime) += secondStep;
            partition = i+1;
            strcpy(partitions[i].code, filename);
            fprintf(logFile, "%d, %d, partition %d marked as occupied\n", *currentTime, thirdStep, i+1); 
            (*currentTime) += thirdStep;
       }
    }
    
    fprintf(logFile, "%d, %d, updating PCB with new information\n", *currentTime, fourthStep);
    (*currentTime) += fourthStep;

    if(strcpy(pcbTable[pcbElements-1].program_name, "init")){
        strcpy(pcbTable[pcbElements-1].program_name,filename);
        pcbTable[pcbElements-1].size = programs[execCalls-1].program_size;
        pcbTable[pcbElements-1].memory_partition = partition;
    } else {
        strcpy(pcbTable[pcbElements].program_name, filename);
        pcbTable[pcbElements].size = programs[execCalls-1].program_size;
        pcbTable[pcbElements-1].memory_partition = partition;

    }

    fprintf(logFile, "%d, %d, scheduler called\n", *currentTime, remaining);
    (*currentTime) += remaining;

    simulateOutput(system_status);

    fprintf(logFile, "%d, 1, IRET\n", *currentTime); //return from the interrupt
    (*currentTime) += 1;

    execCalls++;
}

void simulateOutput(FILE *logFile){
    fprintf(system_status, "!-----------------------------------------------------------!\n");
    fprintf(system_status, "Save Time: %d ms\n", currentTime);
    fprintf(system_status, "+--------------------------------------------+\n");
    fprintf(system_status, "| PID | Program Name | Partition Number | Size |\n");
    fprintf(system_status, "+--------------------------------------------+\n");

    for (int i = 0; i < 6; i++)
    {
        if(pcbTable[i].pid != 0){
            fprintf(system_status,"| %2d |, |%12s | %15d | %4d |\n", pcbTable[i].pid, 
            pcbTable[i].program_name, 
            pcbTable[i].memory_partition, 
            pcbTable[i].size);
        }
    }
    fprintf(system_status, "+--------------------------------------------+\n");
    fprintf(system_status, "!-----------------------------------------------------------!\n");
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

void runSimulation(struct Event *traceEvents, int eventCount, struct VectorTableEntry *vectorTable, FILE *logFile){
    //Loop through all the events in the trace
    for(int i = 0; i < eventCount; i++){
        printf("Processing %s event with ISR number: %d\n", traceEvents[i].type ,traceEvents[i].event_number); // Debugging
        if (strcmp(traceEvents[i].type, "CPU") == 0){
            fprintf(logFile, "%d, %d, CPU\n", currentTime, traceEvents[i].duration);
            currentTime += traceEvents[i].duration;
        }

        //Handle SYSCALL
        else if (strncmp(traceEvents[i].type, "SYSCALL", 7) == 0){
            printf("Processing SYSCALL event with ISR number: %d\n", traceEvents[i].event_number); // Debugging
            simulateISR(logFile, &currentTime, traceEvents[i].event_number, traceEvents[i].duration, vectorTable);
        }

        //Handle END_IO
        else if (strncmp(traceEvents[i].type , "END_IO", 6) == 0){
            printf("Processing END_IO event with ISR number: %d\n", traceEvents[i].event_number); // Debugging
            simulateEndIO(logFile, &currentTime, traceEvents[i].event_number, traceEvents[i].duration, vectorTable);
        }
        
        //Handle FORK
        else if (strncmp(traceEvents[i].type , "FORK", 4) == 0){
            printf("Processing FORK event with ISR number: %d\n", 2); // Debugging
            simulateFork(logFile,&currentTime, 2, traceEvents[i].duration, vectorTable);
        }

        //Handle EXEC
        else if (strncmp(traceEvents[i].type, "EXEC", 4) == 0) {
            printf("Processing EXEC event with ISR number: %d\n", 3); // Debugging
            simulateExec(logFile, &currentTime, 3, traceEvents[i].duration, vectorTable, traceEvents[i].file_name);

            struct Event programEvents[MAX_EVENTS];
            char program[15];
            sprintf(program, "%s.txt", traceEvents[i].file_name);
            int programEventCount = readTraceFile(program, programEvents);
            printf("Starting simulation for program: %s\n", traceEvents[i].file_name);
            runSimulation(programEvents, programEventCount, vectorTable, logFile);
            printf("Returning to main trace after completing program: %s\n", traceEvents[i].file_name);
        }
    }
}

//main code with shell call arguments that calls trace files and outputs to execution files
int main(int argc, char *argv[]) {
    if (argc != 3){
        printf("Usage: %s <shell_file> <external_files>\n", argv[0]);
        return -1;
    }

    const char *traceFile = getTraceFileName(argv[1]);
    const char *outputFileName = getOutputFileName(argv[1]);
    

    partitions[0].partition_number = 1;
    partitions[0].size = 40;
    strcpy(partitions[0].code, "free");

    partitions[1].partition_number = 2;
    partitions[1].size = 25;
    strcpy(partitions[1].code, "free");

    partitions[2].partition_number = 3;
    partitions[2].size = 15;
    strcpy(partitions[2].code, "free");

    partitions[3].partition_number = 4;
    partitions[3].size = 10;
    strcpy(partitions[3].code, "free");

    partitions[4].partition_number = 5;
    partitions[4].size = 8;
    strcpy(partitions[4].code, "free");

    partitions[5].partition_number = 6;
    partitions[5].size = 2;
    strcpy(partitions[5].code, "free");

    pcbTable[0].pid = 1;
    pcbTable[0].memory_partition = 6;
    strcpy(pcbTable[0].program_name, "init");
    pcbTable[0].size = 1;

    pcbElements++;

    strcpy(partitions[5].code, "init");

    if(traceFile == NULL)
    {
        return -1; //error extracting the trace file name
    }

    if(runShellScript(argv[1]) != 1){
        return -1; //error running the shell script
    }

    struct Event traceEvents[MAX_EVENTS]; //to hold the traced events
    struct VectorTableEntry vectorTable[VECTOR_TABLE_SIZE]; //for the vector table

    readVectorTable("vector_table_101263973_101265285.txt", vectorTable);

    int eventCount = readTraceFile(traceFile, traceEvents);
    if (eventCount < 0) {
        return -1; //Error
    }

    int externalFiles = readExternalFiles(argv[2], programs);
    if (externalFiles < 0){
        return -1;
    }

    FILE *executionFile = fopen(outputFileName, "w");
    if (executionFile == NULL) {
        perror("Error opening execution file");
        fclose(executionFile); // Make sure to close any open files before returning
        return 1;
    }

    system_status = fopen("system_status_101263973_101265285.txt", "w");
    if(!system_status){
        printf("Cannot open the system status file: system_status_101263973_101265285.txt");
        return -1;
    }

    simulateOutput(system_status);
    
    runSimulation(traceEvents, eventCount, vectorTable, executionFile);

    fclose(executionFile);
    return 0;
} 
