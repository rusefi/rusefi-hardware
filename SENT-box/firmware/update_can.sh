#!/bin/bash

set -e

../../ext/openblt/Host/BootCommander.exe -s=xcp -t=xcp_can -d=peak_pcanusb -t1=1000 -t3=2000 -t4=10000 -t5=1000 -t7=2000 build/sent_box_blue_pill.srec