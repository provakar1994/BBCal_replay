#!/bin/sh

## Edits
# P. Datta <pdbforce@jlab.org> Created 11-14-2021

## Usage
# This script generates calibrated HVs for BB Shower and
# PreShower using cosmic data. Example execution: 
#./get_calibrated_hv.sh <nrun> <desired_trigger_amplitude>

# Validating the number of arguments provided
if [[ "$#" -ne 2 ]]; then
    echo -e "\n--!--\n Illegal number of arguments!!"
    echo -e " This script expects two arguments: <nrun> <desired_trigger_amplitude(mV)> \n"
    exit;
fi

# Going to work directory
cd macros

inputHV=run_$1_hv.set
slowc_dir=/adaqfs/home/aslow/JAVA/slowc_bbcal/BBCAL/hv_set

# Let's check whether the input HV file for the recent run exists or not. 
HV_FILE1=${PWD}/hv_set/$inputHV
HV_FILE2=$slowc_dir/$inputHV
if [[ -f  $HV_FILE2 && ! -f $HV_FILE1 ]]; then
    while true; do
	echo -e " \n ----<< Found '$inputHV' in adaqsc but not in aonlX! >>---- \n "
	read -p " Want to copy the file to aonlX? [Y/N] " yn
	case $yn in
	    [Yy]*)
		scp aslow@adaqsc:$HV_FILE2 ${PWD}/hv_set/;
		break; ;;
	    [Nn]*)
		echo -e "\n--!--\n HV file for run "$1" doesn't exist!!"
		echo " Please create the proper hv_set/$inputHV file and try again."
		echo -e "--!--\n" ;
		exit;
	esac
    done
fi
if [[ ! -f  $HV_FILE1 && $HV_FILE2 ]]; then
    while true; do
	echo "-----"
	echo " Was the defalut 25 mV HV setting used for this run? [Y/N] "
	echo " [i.e. hv_set/hv_updated_sh_ps_25mV_11_6_21.set] " 
	read -p "" yn
	case $yn in
	    [Yy]*) # If yes, make a copy of the default settings
		cp hv_set/hv_updated_sh_ps_25mV_11_6_21.set hv_set/run_$1_hv.set; 
		echo -e "-----\n hv_set/'$inputHV' created.\n-----\n"; 
		break; ;;
	    [Nn]*) 
		echo -e "\n--!--\n HV file for run "$1" doesn't exist!!"
		echo " Please create the proper hv_set/'$inputHV' file and try again."
		echo -e "--!--\n" ;
		exit;
	esac
    done
fi

# Calculating preshower HVs
echo -e "\n ----=====<< Getting PreShower HVs >>=====---- \n"
sleep 2
root -l -b -q 'PreShower_macros/ps_HVUpdate_cosmic.C('$1','$2')'
evince plots/ps_hv_calib_run_$1_$2mV.pdf
echo -e "\n"

read -p " ---> PreShower HVs Looked OK? [Y/N] " yn
if [[ "$yn" =~ N|n ]]; then
    echo -e "   *** Please fix the issue and try again. ***\n "
    exit;
fi

sleep 2
# Calculating shower HVs
echo -e "\n ----=====<< Getting Shower HVs >>=====---- \n"
sleep 2
root -l -b -q 'Shower_macros/sh_HVUpdate_cosmic.C('$1','$2')'
evince plots/sh_hv_calib_run_$1_$2mV.pdf
echo -e "\n"

read -p " ---> Shower HVs Looked OK? [Y/N] " yn
if [[ "$yn" =~ N|n ]]; then
    echo -e "   *** Please fix the issue and try again. ***\n "
    exit;
fi

date=$(date '+%m_%d_%Y')
shcalibHV=sh_hv_calib_run_$1_$2mV.set
pscalibHV=ps_hv_calib_run_$1_$2mV.set
outputHV=hv_calibrated_run_$1_$2mV

sleep 2
# Combine the HVs to get new settings
echo -e "\n ----=====<< Combining Shower and PreShower HVs >>=====---- \n"
sleep 2
root -l -b -q 'Combined_macros/Combine_HV.C(0,"'$shcalibHV'","'$pscalibHV'","'$outputHV'")'

sleep 1
echo -e "\n ----=====<< Calibrated HV file has been generated. >>=====---- \n"
read -p " --->  Want to copy the file to the adaqsc machine? [Y/N] " yn
case $yn in
    [Yy]*) # If yes, copy the file to adaqsc machine
	echo -e "\n ----=====<< Coping Generated HV file to '$slowc_dir' >>=====---- \n";
	scp hv_set/$outputHV'_'$date.set aslow@adaqsc:$slowc_dir;
	sleep 1;
	evince plots/$outputHV'_'$date.pdf;
	break; ;;
    [Nn]*) 
 	evince plots/$outputHV'_'$date.pdf;
	break;
esac
