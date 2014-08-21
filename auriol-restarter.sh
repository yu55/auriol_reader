#!/bin/sh

cd /home/pi/auriol_pluviometer_reader

# Kill 1
#screen -X -S | grep starbound kill

# Kill 2
pkill -9 auriol-pluviometer-reader

# Change directory
#cd /home/steam/starbound/linux64

DATE=$(date +"%Y%m%d%H%M")
mv output-auriol.log output-auriol_$DATE.log

# Start the server again
screen -S auriol -d -m ./auriol-start.sh
