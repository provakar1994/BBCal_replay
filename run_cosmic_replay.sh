#!/bin/sh

## Edits
# P. Datta <pdbforce@jlab.org> Created 11-14-2021

## Usage
# This macro replays the cosmic run for 
# BB Shower and PreShower. Example execution: 
#./run_cosmic_replay.sh <nrun> <nevent>

## Choice of experiment
GMn="e1209019"
GEn="e1209016"

module purge
module load analyzer
module list

cd replay
analyzer -b -q 'replay_BBCal.C+('$1','$2',"'$GEn'")'

