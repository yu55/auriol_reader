#!/bin/sh

cd /home/pi/auriol_pluviometer_reader

# Kill
killall auriol-pluviometer-reader

DATE=$(date +"%Y%m%d%H%M")
mv output-auriol.log output-auriol_$DATE.log

# Start the server again
screen -S auriol -d -m ./auriol-start.sh
