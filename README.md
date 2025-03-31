# Queuing Simulators

Some queing simulations written in C. Compilation tested using gcc 5.4.
Instructions on how to compile and run the simulators is written on each file.

This repository includes the following models for simulation:
* M/M/1
* M/M/c-1
* M/M/c-2
* M/M/c/k

## Author

Lucas German Wals Ochoa

## Extensions
1) The M/M/c-1 uses a random assignment of jobs to servers
2) The M/M/c-2 uses a random assignment of jobs to servers (but it only considers the idle cores not active)
3) Output more statistics (core idle time, package idle time, arrivals per core, idle time)

## To Fix
1) Fix output of simulators (make it uniform).
