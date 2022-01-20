#!/bin/csh

echo "\n"
echo " ** Welcome to the home of BBCal analysis. Enjoy! ** "
echo "\n"

source /work/halla/sbs/datta/SBSOFFLINE/install/bin/sbsenv.csh

setenv SBS_REPLAY /work/halla/sbs/SBS_REPLAY/SBS-replay
setenv DB_DIR $SBS_REPLAY/DB
setenv DATA_DIR /cache/mss/halla/sbs/raw

setenv OUT_DIR /volatile/halla/sbs/datta/GMN_REPLAYS/rootfiles
setenv LOG_DIR /volatile/halla/sbs/datta/GMN_REPLAYS/logs
setenv ANALYZER_CONFIGPATH $SBS_REPLAY/replay
