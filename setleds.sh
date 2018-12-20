#!/bin/bash

## Here are my 4 disk paths in sysfs
HD0="/sys/class/scsi_host/host0/device/target0:0:0/0:0:0:0"
HD1="/sys/class/scsi_host/host0/device/target0:0:1/0:0:1:0"
HD2="/sys/class/scsi_host/host1/device/target1:0:0/1:0:0:0"
HD3="/sys/class/scsi_host/host1/device/target1:0:1/1:0:1:0"

## Blue LEDs in sysfs
LEDB0="/sys/class/leds/hpex47x:blue:hdd0/brightness"
LEDB1="/sys/class/leds/hpex47x:blue:hdd1/brightness"
LEDB2="/sys/class/leds/hpex47x:blue:hdd2/brightness"
LEDB3="/sys/class/leds/hpex47x:blue:hdd3/brightness"

## Red LEDs in sysfs
LEDR0="/sys/class/leds/hpex47x:red:hdd0/brightness"
LEDR1="/sys/class/leds/hpex47x:red:hdd1/brightness"
LEDR2="/sys/class/leds/hpex47x:red:hdd2/brightness"
LEDR3="/sys/class/leds/hpex47x:red:hdd3/brightness"

## First lets check if all drives are responding to hdparm
DRIVES_OK=1
for drive in $(ls /dev/disk/by-id/ata-*|grep -v part)
do
/sbin/hdparm -i $drive >/dev/null 2>&1 || DRIVES_OK=0
done
if [ 0 -eq $DRIVES_OK ]
then
  # not all drives are talking with hdparm, rescan needed
  echo 0 0 0 > /sys/class/scsi_host/host0/scan
  echo 0 1 0 > /sys/class/scsi_host/host0/scan
  echo 0 0 0 > /sys/class/scsi_host/host1/scan
  echo 0 1 0 > /sys/class/scsi_host/host1/scan
fi

# set default color (no drive)
BLUE0=0
BLUE1=0
BLUE2=0
BLUE3=0

RED0=0
RED1=0
RED2=0
RED3=0

# check each drive using sysfs. Set LED to blue if disk is ok
if [ -f  "$HD0/state" ]
then
cat "$HD0/state" | grep -q running && BLUE0=255
fi
if [ -f  "$HD1/state" ]
then
cat "$HD1/state" | grep -q running && BLUE1=255
fi
if [ -f  "$HD2/state" ]
then
cat "$HD2/state" | grep -q running && BLUE2=255
fi
if [ -f  "$HD3/state" ]
then
cat "$HD3/state" | grep -q running && BLUE3=255
fi

if [ -f  "$LEDB0" ]
then
echo $BLUE0 > "$LEDB0"
fi
if [ -f  "$LEDB1" ]
then
echo $BLUE1 > "$LEDB1"
fi
if [ -f  "$LEDB2" ]
then
echo $BLUE2 > "$LEDB2"
fi
if [ -f  "$LEDB3" ]
then
echo $BLUE3 > "$LEDB3"
fi

if [ -f  "$LEDR0" ]
then
echo $RED0 > "$LEDR0"
fi
if [ -f  "$LEDR1" ]
then
echo $RED1 > "$LEDR1"
fi
if [ -f  "$LEDR2" ]
then
echo $RED2 > "$LEDR2"
fi
if [ -f  "$LEDR3" ]
then
echo $RED3 > "$LEDR3"
fi

exit 0
