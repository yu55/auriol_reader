#!/bin/bash
service=auriol-pluviometer-reader

if (( $(ps -ef | grep -v grep | grep $service | wc -l) > 0 ))
then
echo "$service is running!!!"
else
( cd /home/pi/auriol_pluviometer_reader ; ./auriol-restarter.sh )
fi

