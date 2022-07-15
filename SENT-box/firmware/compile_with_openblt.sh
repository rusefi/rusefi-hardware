#!/bin/bash

BOARD=blue_pill USE_OPENBLT=yes \
USE_OPT="-O0 -ggdb -fomit-frame-pointer -falign-functions=16 -fsingle-precision-constant" \./common_make.sh
