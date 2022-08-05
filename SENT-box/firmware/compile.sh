#!/bin/bash

BOARD=blue_pill \
USE_OPT="-O2 -ggdb -fomit-frame-pointer -falign-functions=16 -fsingle-precision-constant" \./common_make.sh