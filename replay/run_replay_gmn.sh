#!/bin/sh

## Edits
# P. Datta <pdbforce@jlab.org> Created 11-14-2021

## Usage
# This macro replays the Production run for 
# BBCal analysis. Example execution: 
#./run_replay_gmn.sh <nrun> <nevent> <firstseg> <maxseg>

module purge
module load analyzer

nrun=$1
nevents=$2
fseg=$3
mseg=$4

analyzer -b -q 'replay_gmn_PD.C+('$nrun','$nevents',0,"e1209019",'$fseg','$mseg',0)'
