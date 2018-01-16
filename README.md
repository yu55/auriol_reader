# auriol-reader
This repository contains AURIOL H13726 / Ventus W155 weather stations radio transmissions decoder application for Raspberry Pi. Application obtains data through a 433.92 MHz RF wireless receiver module (AUREL RX-4MM5++/F, simple chinese XY-MK-5V or other similar) connected to GPIO pin, decodes this data, prints it to `stdout` and saves in SQLite database.

![auriol-reader-screenshot.png](auriol-reader-screenshot.png?raw=true "View of data received from AURIOL H13726 weather station via auriol-reader")

## Repository layout
* `doc` - additional documentation
* `reader` - decoder application
* `scripts` - some scripts to make `auriol-reader` work continuously and start after reboot
* `www` - some examples of primitive web pages showing data received by `auriol-reader`

## Installation
* This project uses Wiring Pi library (http://wiringpi.com/) so it should be installed first.
* Secondly the `libsqlite3-dev` package is required.
* Clone this repository, `cd` into `reader` directory, execute `make` and project should compile.

## Running
* connect receiver module output signal to Raspberry GPIO27
* execute application with root privileges: `sudo ./auriol-reader`
* observe decoded data on standard output and in `/var/local/auriol-db.sl3` database

## Additional notes
* If different GPIO pin must be used check how pins are numbered in Wiring Pi (http://wiringpi.com/pins/) and modify constant `RECIEVE_PIN` in `reader/auriol-reader.c` source code file (e.g. GPIO27 is pin 2 in Wiring Pi).
* If nothing is visible on `stdout` that means decoder cannot recognize incoming impulses and some calibration might be needed - please check `reader/auriol-reader.c` for constants `SYNCHRO_LENGTH`, `SEPARATOR_LENGTH`, `ZERO_LENGTH`, `ONE_LENGTH`, `LENGTHS_MARGIN`. Setting these constants to proper values is critical for the decoder to be able to recognize data in incoming transmissions from weather station instruments. Note that `auriol-reader` consumes 100% CPU (because of constantly polling GPIO pin connected to receiver hardware) and running other process with heavy CPU usage will result in slower GPIO polling so the decoder simply won't recognize data for given constants configuration. You may also want to tweak these values on newer, faster Raspberry Pi models (faster than my own Model B Revision 2.0).

