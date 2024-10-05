#Authors: Gaetan Fodjo 

import re
import pandas as pd
import sys

# categorize the lines according to their content(CPU work, I/O work or Overhead)
def categorize_line(line):
    if "CPU" in line or "SYSCALL" in line:
        return "CPU"
    elif "transfer" in line or "check for" in line or "END_IO" in line:
        return "I/O"
    elif "switch to kernel mode" in line or "context" in line or "find" in line or "load" in line or "IRET" in line or "check priority" in line or "check if masked" in line:
        return "Overhead"
    return None

# Extracting the time, duration and action from the line
def extract_time_and_action(line):
    # Matching the the pattern of our execution file: r for raw string(so we dont confuse with escape characters), \d+ for digits, \s+ for whitespace, and .+ for every other character
    pattern = r"(\d+),\s*(\d+),\s*(.+)"

    match = re.match(pattern,line)
    if match:
        time = int(match.group(1))
        duration = int(match.group(2))
        action = match.group(3)
        return time, duration, action
    
    else:
        print(f"No match found on line: '{line}'")
    return None, None, None

# process execution file line by line, assigning category's and calculating the ration between the 3 cateogories
def processing_execution_file(file_path):
    cpu_time = 0
    io_time = 0
    overhead_time = 0

    rows = []
    with open(file_path, 'r') as execution_file:
        for line in execution_file:
            time, duration, action = extract_time_and_action(line)
            if time is not None and duration is not None:
                category = categorize_line(action)
                if category == "CPU":
                    cpu_time += duration
                    #Rows of data for our excel file
                    rows.append([duration, action, time, duration, 0, 0])
                if category == "I/O":
                    io_time += duration
                    rows.append([duration, action, time, 0, duration, 0])
                if category == "Overhead":
                    overhead_time += duration
                    rows.append([duration, action, time, 0, 0, duration])
    
    total_time = cpu_time + io_time + overhead_time

    return rows, cpu_time, io_time, overhead_time, total_time

def write_to_excel(rows, cpu_time, io_time, overhead_time, total_time, output_file):
    df = pd.DataFrame(rows, columns=["Duration", "Name", "Time", "CPU Time", "I/O Time", "Overhead Time"])
    if total_time == 0:
        cpu_ratio = io_ratio = overhead_ratio = 0
    else:
        cpu_ratio = (cpu_time / total_time) * 100
        io_ratio = (io_time / total_time) * 100
        overhead_ratio = (overhead_time / total_time) * 100 
    #Creating total time summary and ratio row
    df.loc[len(df)] = ["", "Total Time", total_time, cpu_time, io_time, overhead_time]
    df.loc[len(df)] = ["", "Ratio", "", f"{cpu_ratio:.2f}%", f"{io_ratio:.2f}%", f"{overhead_ratio:.2f}%"]
    
    df.to_excel(output_file, index=False)
    print(f"Data written to {output_file}")

#COMMAND TO RUN:  
#python execution_analyzer.py ..\\otherTests\\executionX.txt executionX_summary.xlsx for execution files 3-20 corresponding to test 3-20
#python execution_analyzer.py ..\\interrupt-simulator\\executionX.txt executionX_summary.xlsx for execution files 1 and 2.
#YOU CAN WRITE AND SAVE EXCEL SHEETS WHEREVER YOU PLEASE ASLONG U INPUT THE CORRECT PATH
def main():
    if len(sys.argv) != 3:
        print("Usage: pyhton execution anaylyzer.py <execution_file> <execution_summary_excel_file>")
        return

    #Getting input and output files from user input
    input_file = sys.argv[1] 
    output_file = sys.argv[2]

    print(f"Input file: {input_file}")
    print(f"Output file: {output_file}")

    rows, cpu_time, io_time, overhead_time, total_time = processing_execution_file(input_file)
    print("Processing complete")
    write_to_excel(rows, cpu_time, io_time, overhead_time, total_time, output_file)
    print(f"Writen to {output_file}")

if __name__ == "__main__":
    print("script is running")
    main()

    

