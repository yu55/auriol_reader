#!/usr/local/bin/gnuplot -persist
set terminal png size 640,480 enhanced font "Helvetica,10"
set output '/var/www/tmp/output.png'

set title "Temperatura za ostanie 24 godziny"
unset multiplot
unset key
set style data lines
set grid ytics lc rgb "#bbbbbb" lw 1 lt 0
set grid xtics lc rgb "#bbbbbb" lw 1 lt 0
set datafile separator "|"
set xdata time
set timefmt "%Y-%m-%d %H:%M:%S"
set format x "%H:%M"

plot "< sqlite3 database.sl3  \"SELECT created, temperature FROM anemometer WHERE created > datetime('now','localtime','-1 day')\"" using 1:2

