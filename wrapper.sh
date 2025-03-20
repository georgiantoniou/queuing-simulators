#!/bin/bash
####################################################################
#               Wrapper for M/M/c Queue Simulator
####################################################################
# Description: This script runs the M/M/c model for different values 
# of lambda, # μ and c. It saves the output to a directory passed as 
# an argument.
#-------------------------------------------------------------------
# Execution: 
# ./wrapper lambda:min,max,step mean:min,max,step c:min,max,step
# ./wrapper 0.010,0.00  1:10:1
#-------------------------------------------------------------------


if [[ -z $1 || -z $2 || -z $3 ]]; then 
    echo "Usage: ./wrapper.sh <runs> <duration>"
    exit;
fi







