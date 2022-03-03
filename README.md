# BBCal_replay
This repository contains calibration and analysis scripts for BigBite Calorimeter (BBCal) those are being used for JLab Hall A SBS collaboration experiments.

### Brief description of BBCal:
BigBite calorimeter is made of two parts, Shower (SH) and PreShower (PS). SH detector is composed of 189 modules, each composed of a 8.5x8.5x37cm lead-glass blocks readout by a PMT. The 189 modules are laid out in 27 rows of 7 blocks all facing the spectrometer z-axis. Similarly, PS detector is composed of 52 modules laid out in 26 rows of 2 blocks all facing the spectrometer z-axis. Each PS module is made of 9x9x37cm lead-glass block readout by a PMT. 

### List of directories:
The names of the directories are self-explanatory.

- `golden` - Contains alpha values for all SH and PS PMTs. 
- `replay` - Contains machiary for BBCal standalone replay for SBS GMn data. The most up-to-date replay machinary for SBS GMn can be found in [SBS-replay](https://github.com/JeffersonLab/SBS-replay). NOTE: It is necessary to modify the .rootrc and the environment setup files depending on the user's working directory and software environmets for the replay to work properly.  
