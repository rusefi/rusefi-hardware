#!/bin/bash

set -e

if [ ! "$USE_OPENBLT" ]; then
  USE_OPENBLT=no
fi

if [ $USE_OPENBLT = "yes" ]; then
  cd openblt

  echo ""
  echo "Building bootloader"
  make clean
  make -j12 BOARD=${BOARD} || exit 1

  # back to board dir
  cd ..
fi

# cd ../..

echo ""
echo "Build application"
export EXTRA_PARAMS="-DECHO_UART=TRUE"
make clean
make -j12 BOARD=${BOARD} || exit 1

DELIVER_DIR=deliver/${BOARD}
mkdir -p ${DELIVER_DIR}
rm -f ${DELIVER_DIR}/*

cd ../..

if uname | grep "NT"; then
  HEX2DFU=./ext/encedo_hex2dfu/hex2dfu.exe
else
  HEX2DFU=./ext/encedo_hex2dfu/hex2dfu.bin
fi
chmod u+x $HEX2DFU

echo ""
echo "Creating deliveries:"

if [ $USE_OPENBLT = "yes" ]; then
  echo "Srec for CAN update"
  cp -v SENT-box/firmware/build/sent_box_${BOARD}.srec SENT-box/firmware/${DELIVER_DIR}/sent_box_update.srec

  echo ""
  echo "Invoking hex2dfu for incremental SENT-box image (for DFU util)"
  $HEX2DFU -i SENT-box/firmware/build/sent_box_${BOARD}.hex -C 0x1C -o SENT-box/firmware/${DELIVER_DIR}/sent_box_update.dfu

  echo ""
  echo "Invoking hex2dfu for OpenBLT (for DFU util)"
  $HEX2DFU -i SENT-box/firmware/openblt/bin/openblt_${BOARD}.hex -o SENG-box/firmware/${DELIVER_DIR}/openblt.dfu

  echo ""
  echo "OpenBLT bin (for DFU another util)"
  cp -v SENT-box/firmware/openblt/bin/openblt_${BOARD}.bin SENT-box/firmware/${DELIVER_DIR}/openblt.bin

  echo ""
  echo "Invoking hex2dfu for composite OpenBLT+SENT image (for DFU util)"
  $HEX2DFU -i SENT-box/firmware/openblt/bin/openblt_${BOARD}.hex -i SENT-box/firmware/build/sent_box_${BOARD}.hex -C 0x1C -o SENT-box/firmware/${DELIVER_DIR}/sent_box_openblt.dfu -b SENT-box/firmware/${DELIVER_DIR}/sent_box_openblt.bin 
else
  echo "Bin for raw flashing"
  cp SENT-box/firmware/build/sent_box_${BOARD}.bin SENT-box/firmware/${DELIVER_DIR}/sent_box.bin

  echo "Invoking hex2dfu for DFU file"
  $HEX2DFU -i SENT-box/firmware/build/sent_box_${BOARD}.hex -o SENT-box/firmware/${DELIVER_DIR}/sent_box.dfu
fi

echo ""
echo "${DELIVER_DIR} folder content:"
ls -l SENT-box/firmware/${DELIVER_DIR}
