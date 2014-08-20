#!/usr/local/bin/gnuplot -persist
set terminal png size 640,480 enhanced font "Helvetica,8"
set output '/var/www/tmp/temperature24h.png'

set title "Temperatura powietrza za ostanie 24 godziny"
unset multiplot
unset key
set style data lines
set grid ytics lc rgb "#bbbbbb" lw 1 lt 0
set grid xtics lc rgb "#bbbbbb" lw 1 lt 0
set datafile separator "|"
set xdata time
set timefmt "%Y-%m-%d %H:%M:%S"
set format x "%H:%M"
set x2tics
set x2data time
set format x2 "%H:%M"
unset mx2tics
set y2tics

plot "< sqlite3 database.sl3  \"SELECT created, temperature FROM anemometer WHERE created > datetime('now','localtime','-1 day')\"" using 1:2

