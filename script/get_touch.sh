#!/bin/bash

DEV=`sudo ./touch_mode | grep TouchScreen | awk '{print $6}'`
DEV_INPUT=`sudo ./lsevent | grep $DEV | awk '{print $8}'`
echo $DEV_INPUT