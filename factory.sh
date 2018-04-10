#!/bin/sh
FirmwareDir="/Volumes/esp-open-sdk/rtos_sdk_esp8266"
AppDir="/Users/huxinbang/code/github/iotsuite-esp8266-rtos"
cd "$FirmwareDir" 
#port=/dev/ttyUSB0
port=`ls /dev/cu*|grep serial`
# port=/dev/cu.wchusbserial1420
# port=/dev/cu.wchusbserial14240
if [ ! -c $port ]; then
   port=/dev/ttyUSB1
fi
if [ ! -c $port ]; then
   echo "No device appears to be plugged in.  Stopping."
fi
printf "Writing AT firmware to the ESP8266 board in 3..."
sleep 1; printf "2..."
sleep 1; printf "1..."
sleep 1; echo "done."
echo "Erasing the flash first"
"esptool.py" --port $port erase_flash
 
"esptool.py" --chip esp8266 --port $port \
   write_flash -fm dio -ff 20m -fs detect \
   0x0000 "$FirmwareDir/bin/boot_v1.7.bin" \
   0x01000 "$AppDir/bin/upgrade/user1.2048.new.5.bin" \
   0x3fc000 "$FirmwareDir/bin/esp_init_data_default.bin"  \
   0x7e000 "$FirmwareDir/bin/blank.bin"  \
   0x3fe000 "$FirmwareDir/bin/blank.bin"
 
echo "Check the boot by typing: miniterm.py $port 74880"
echo " and then resetting.  Use Ctrl-] to quit miniterm,"


