```markdown
# Interrupt Simulator Project

## Overview
The Interrupt Simulator is a project developed to simulate the behavior of an operating system’s interrupt handling mechanism
and process scheduling. It allows users to simulate and analyze how interrupts are processed—from when they are triggered to
when they are handled by the appropriate Interrupt Service Routine (ISR)—while also managing process control and memory allocation.

This project has been enhanced to incorporate Process Control Block (PCB) Scheduling, where each process in the system is
represented by a PCB that contains information about the process, such as its ID, program name, allocated memory partition,
and remaining CPU time. This allows for dynamic scheduling of processes and more accurate simulation of system behavior under
different conditions.

Additionally, the simulator includes Fixed Partition Memory Management, where processes are allocated memory partitions based
on available space, and memory usage is tracked throughout the simulation.

Developed in C, this project demonstrates proficiency in low-level systems programming, including handling input/output operations,
managing interrupts, scheduling processes, and simulating memory management techniques. It provides an in-depth view of how an
operating system handles processes, memory, and interrupts in a controlled environment.

## Features
- **Interrupt Handling Simulation**: Simulates various interrupt types such as CPU tasks, system calls, and I/O interrupts.
- **Vector Table Implementation**: Utilizes a vector table to store and manage ISRs for different interrupts.
- **End I/O Event Handling**: Specialized handling of **END_IO** events, with accurate time and priority checks.
- **Traceable Outputs**: Outputs are logged in files for detailed analysis, including steps such as switching to kernel mode, saving context, and executing ISRs.
- **Extensible Design**: Supports additional interrupt types and functionalities, making it suitable for further development.

## Skills Highlighted
- **C Programming**: Implemented efficient and organized C code, demonstrating proficiency in low-level programming.
- **Systems Programming**: Simulated core operating system behaviors such as interrupt handling, context switching, and memory management.
- **File I/O**: Managed input from trace files and produced execution logs.
- **Algorithmic Thinking**: Used sorting and simulation techniques to process interrupts in order and manage time and priority correctly.
- **Collaboration**: Worked with teammates and used professional development practices, including writing well-structured, modular code.

## File Structure
- **interrupts.c**: The main source file for the simulator, containing the logic for handling interrupts and processing trace files.
- **interrupts.h**: Header file defining the structures and function declarations for the simulator.
- **test1.sh**: Shell script to compile and run the simulator with `trace1.txt` as input.
- **test2.sh**: Shell script to compile and run the simulator with `trace2.txt` as input.
- **trace1.txt**: Example trace file used as input to simulate a series of interrupts.
- **trace2.txt**: Another example trace file for a different simulation scenario.
- **vector_table.txt**: Contains the vector table that maps interrupt numbers to memory addresses for ISR handling.
- **execution1.txt**: Output log file generated from running the simulator with `trace1.txt`.
- **execution2.txt**: Output log file generated from running the simulator with `trace2.txt`.

## How to Run the Simulation

### Prerequisites
- **GCC Compiler**: Ensure that `gcc` is installed on your system.
- **Shell Environment**: A UNIX-like environment is recommended (Linux or macOS).

### Steps
1. **Clone the Project**: 
   Download or clone the project folder.
   
2. **Compile the Program**: 
   Use one of the provided shell scripts to compile the program. 
   For example, to use `trace1.txt`:
   ```bash
   ./test1.sh
   ```
   This will compile the program and run the simulation using the trace file.

3. **Run the Simulation**: 
   After running the shell script, the simulator will output a detailed log of the interrupt handling process to `execution1.txt` or `execution2.txt`, depending on the trace file used.

### Example Execution
To run the simulator with `trace1.txt`, simply execute:
```bash
./test1.sh
```
The simulation log will be saved in `execution1.txt`.

To run with `trace2.txt`:
```bash
./test2.sh
```
The log will be saved in `execution2.txt`.

## Key Output Log Entries
1. **Priority Check**: Logs when the interrupt priority is checked.
2. **Context Switch**: Indicates when the system switches to kernel mode and saves the current context.
3. **Memory Access**: Logs details about memory accesses, including fetching the appropriate ISR address.
4. **Execution**: Records the execution phases of the ISR.
5. **End I/O Handling**: Details the steps taken to handle an END_IO event, including time tracking and memory vector updates.

## Example Output
```
191, 1, check priority of interrupt
192, 1, check if masked
193, 1, switch to kernel mode
194, 3, context saved
197, 1, find vector 5 in memory position 0x000A
198, 1, load address 0X0069 into the PC
199, 248, END_IO
```
```
