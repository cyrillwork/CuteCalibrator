#!/bin/bash
#all credit to Mogwai. Script taken from url: https://wiki.archlinux.org/index.php/Talk:Calibrating_Touchscreen

#device=$(xinput_calibrator --list)
#device=$(sed -n 's/.*Device\s\"\(.*\)\".*/\1/p' <<< $device)

#reset xinput matrix
xinput set-prop 8 'Coordinate Transformation Matrix' 0.5 0 0 0 1 0 0 0 1
exit 0

out=$(xinput_calibrator)

device_name=$(sed -n 's/.*MatchProduct\"\s\"\([0-9]*\).*/\1/p' <<< $out)

wtot=$(sed -n 's/.*max_x=\([0-9]*\).*/\1/p' <<< $out)
htot=$(sed -n 's/.*max_y=\([0-9]*\).*/\1/p' <<< $out)

minx=$(sed -n 's/.*MinX\"\s\"\([0-9]*\).*/\1/p' <<< $out)
maxx=$(sed -n 's/.*MaxX\"\s\"\([0-9]*\).*/\1/p' <<< $out)
miny=$(sed -n 's/.*MinY\"\s\"\([0-9]*\).*/\1/p' <<< $out)
maxy=$(sed -n 's/.*MaxY\"\s\"\([0-9]*\).*/\1/p' <<< $out)

wtouch=$(bc <<< "$maxx - $minx")
htouch=$(bc <<< "$maxy - $miny")

c0=$(bc -l <<< "$wtot / $wtouch")
c1=$(bc -l <<< "-$minx / $wtouch")
c2=$(bc -l <<< "$htot / $htouch")
c3=$(bc -l <<< "-$miny / $htouch")

tf_matrix="$c0 0 $c1 0 $c2 $c3 0 0 1"
echo $tf_matrix > /build/tfmatrix

#alter the setting for now
xinput set-prop "$device" 'Coordinate Transformation Matrix' $tf_matrix
