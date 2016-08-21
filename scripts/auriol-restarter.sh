#!/bin/sh

cd /home/pi/repositories/auriol_reader/scripts

# Kill
killall auriol-reader

#DATE=$(date +"%Y%m%d%H%M")
#mv output-auriol.log output-auriol_$DATE.log

# Start the server again
screen -S auriol -d -m ./auriol-start.sh
