#! /bin/bash
gcc interrupts.c -I interrupts.h -o sim

./sim trace.txt execution.txt
