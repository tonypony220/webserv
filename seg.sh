CURDIR=$(pwd)
cd ~
cd Library/Logs/DiagnosticReports/
RESULT=$(ls -tp | grep -v /$ | head -1)
head -50 "$RESULT"
cd $CURDIR
