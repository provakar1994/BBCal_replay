#!/bin/sh

## Edits
# P. Datta <pdbforce@jlab.org> Created 11-14-2021

## Usage
# This script generates calibrated HVs for BB Shower and
# PreShower using cosmic data. Example execution: 
#./get_calibrated_hv.sh <nrun> <desired_trigger_amplitude>

cd macros

# Let's check whether the HV file for the recent run
# exists or not. 
HV_FILE=${PWD}/hv_set/run_$1_hv.set
if [[ ! -f  "$HV_FILE" ]]; then
    while true; do
	echo "-----"
	echo " Was the defalut 25 mV HV setting used for this run? [Y/N] "
	echo " [i.e. hv_set/hv_updated_sh_ps_25mV_11_6_21.set] " 
	read -p "" yn
	case $yn in
	    [Yy]*) # If yes, make a copy of the default settings
		cp hv_set/hv_updated_sh_ps_25mV_11_6_21.set hv_set/run_$1_hv.set; 
		echo -e "-----\n hv_set/run_"$1"_hv.set created.\n-----\n"; 
		break; ;;
	    [Nn]*) 
		echo -e "--!--\n HV file for run "$1" doesn't exist!!"
		echo " Please create the proper hv_set/run_"$1"_hv.set file and try again."
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
    echo -e "   *** Please check and try again. ***\n "
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
    echo -e "   *** Please check and try again. ***\n "
    exit;
fi

sleep 2
# Combine the HVs to get new settings
echo -e "\n ----=====<< Combining Shower and PreShower HVs >>=====---- \n"
sleep 2
root -l -b -q 'Combined_macros/Combine_HV.C(0,"sh_hv_calib_run_'$1'_'$2'mV.set","ps_hv_calib_run_'$1'_'$2'mV.set","hv_calibrated_run_'$1'_'$2'mV")'
evince plots/hv_calibrated_run_$1_$2mV.pdf &
