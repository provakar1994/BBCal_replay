#!/bin/sh

## Edits
# P. Datta <pdbforce@jlab.org> Created 11-14-2021

## Usage
# This macro performs the analysis of cosmic data 
# for BB Shower and PreShower. Example execution: 
#./run_cosmic_analysis.sh <nrun> <nevent>

cd macros
echo -e "\n ----=====<< Starting PreShower Analysis >>=====---- \n"
root -l 'PreShower_macros/bbps_cos_cal.C('$1','$2',0)'

read -p " ---> PreShower Peaks Looked OK? [Y/N] " yn
if [[ "$yn" =~ N|n ]]; then
    echo -e "   *** Please check and try again. ***\n "
    exit;
fi
echo -e "\n"

echo -e "\n ----=====<< Starting Shower Analysis >>=====---- \n"
root -l 'Shower_macros/bbsh_cos_cal.C('$1','$2',0)'

read -p " ---> Shower Peaks Looked OK? [Y/N] " yn
if [[ "$yn" =~ N|n ]]; then
    echo -e "   *** Please check and try again. ***\n "
    exit;
fi
echo -e "\n"
