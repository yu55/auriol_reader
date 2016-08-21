#!/bin/bash
service=auriol-reader

if (( $(ps -ef | grep -v grep | grep $service | wc -l) > 0 ))
then
echo "$service is running!!!"
else
( cd /home/pi/repositories/auriol_reader/scripts ; ./auriol-restarter.sh )
fi

