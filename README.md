**BBCal_replay** repository contains calibration and analysis scripts for the BigBite Calorimeter (BBCal), which is an integral part of the BigBite Spectrometer that is being used in the Jefferson Lab's Hall A SBS collaboration experiments to detect scattered electrons. [Procedure wise How-To for BBCAL](https://sbs.jlab.org/cgi-bin/DocDB/public/ShowDocument?docid=313) provides step-by-step guidance to carry out various calibration and analysis procedures for BBCal.

## Contents
1. Brief description of BBCal
2. List of direcotries
3. List of shell scripts
4. Contact

## 1. Brief description of BBCal:
BigBite calorimeter is made of two parts, Shower (SH) and PreShower (PS). SH detector is composed of 189 modules, each composed of a 8.5x8.5x37cm lead-glass blocks readout by a PMT. The 189 modules are laid out in 27 rows of 7 blocks all facing the spectrometer z-axis. Similarly, PS detector is composed of 52 modules laid out in 26 rows of 2 blocks all facing the spectrometer z-axis. Each PS module is made of 9x9x37cm lead-glass block readout by a PMT. 

## 2. List of directories: 
The names of the directories are self-explanatory. Still, below is a brief description of their contents to help understand the user how the entire eco-system is working.

- `golden` - Contains values of alpha parameters for all SH and PS PMTs. _preshower/golden_ps_alpha_11845_11888_10_30mV.txt_ and _shower/golden_sh_alpha_11845_11888_10_30mV.txt_ contain the most up-to-date and robust alpha values for PS and SH PMTs, respectively.  
- `replay` - Contains machiary for BBCal standalone replay for SBS GMn data. The most up-to-date replay machinary for SBS GMn can be found in [SBS-replay](https://github.com/JeffersonLab/SBS-replay). Although, [SBS-offline](https://github.com/JeffersonLab/SBS-offline) contains all the event reconstruction codes. NOTE: It is necessary to modify the replay/.rootrc and the environment setup files depending on the user's working directory and software environmets for the replay to work properly. 
- `macros` - All the calibration scripts and ROOT utility macros can be found here. Most of the scripts have some interdependencies in terms of input or output directory structure and for that to work properly, user should execute all the macros from this directory. Here is a brief description of the contents of several subdirectories _BBCal_replay/macros_ has:
  -  `How-to` - Documentaion on how to execute all the different macros, how to carry out cosmic calibration procedure and then generate gain coefficients, et cetera can be found here.
  -  `Shower_macros` - All analysis scripts that only involve SH, such as SH cosmic calibration, getting new HV values for SH PMTs, et cetera can be found here.
  -  `PreShower_macros` - All analysis scripts that only involve PS, such as PS cosmic calibration, getting new HV values for PS PMTs, et cetera can be found here.
  -  `Combined_macros` - General macros those are applicable to both SH and PS, such as [energy calibration script (with beam)](https://github.com/provakar1994/BBCal_replay/blob/master/macros/Combined_macros/bbcal_eng_calib_w_h2.C), script to calculate ADC gain with cosmic, et cetera, can be found here.
     - `cfg` - _Combined_macros/cfg_ contains all the configuration files for various SBS GMn configurations (taking into consideration different BB & SBS magnet settings within a configuration), which are needed to execute the [energy calibration script (with beam)](https://github.com/provakar1994/BBCal_replay/blob/master/macros/Combined_macros/bbcal_eng_calib_w_h2.C) script.
  -  `Event_disp_macros` - Scripts to look single BBCal events can be found here.
  -  `Output` - Various output .txt files get stored here.
  -  `hist` - All the output .root files get stored here.
  -  `plots` - Generated plots which are savend in .pdf format, get stored here.
  -  `Gain` - ADC gain coefficients and ratios for both SH and PS which serve as both input and output files for the [energy calibration script (with beam)](https://github.com/provakar1994/BBCal_replay/blob/master/macros/Combined_macros/bbcal_eng_calib_w_h2.C), get stored here.
  -  `hv_set` - All the .set files (complatible with JAVA HV GUI) containing HVs for both SH and PS PMTs can be found here.
  -  `Run_list` - Contains configuration specificc run lists.
  -  `hcal` - Has the hcal energy calibration machinary (with beam), which was developed for double checking purposes. 
 
## 3. List of shell scripts:
During GMn run, BBCal cosmic calibration at the start of every SBS configuration turned out to be an absolute must in order to mitigate the SBS and BB fringe field effect. Hence, to automate the process, so that the shift crew can also carry it out, a handful of shell scripts were written. Below is a brief description of their utility. 
  - `run_cosmic_replay.sh` - Starts the replay of the desired cosmic run. Takes two arguments: runnumber and no. of events one desire to replay.
  - `run_cosmic_analysis.sh` - Performs cosmic calibration analysis for PS and then SH, which generate necessary output .txt files those are required for the estimation of calibrated HVs. Takes the same two arugemnts as the run_cosmic_replay.sh script.
  - `get_calibrated_hv.sh` - First generates calibrated HVs for PS, then generates the same for SH and in the end combines the output .set files together to generate an amalgamated calibrated HV .set file for the entire BBCal detector that is ready to be loaded into the HV JAVA GUI.
  - `setup_bbcal.sh` - An example script to setup the environment for replay on the Hall A CH aonlX [where, X=1,2, or 3] machine.
  - `setup_ifarm.sh` - An example script to setup the environment for replay on ifarm.

## 4. Contact:
In case of any questions or concerns please contact the authors,
>Authors: Provakar Datta (UConn), Mark K. Jones (JLab) <br> 
>Contact: <pdbforce@jlab.org> (Provakar)
