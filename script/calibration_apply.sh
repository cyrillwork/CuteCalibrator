#!/bin/bash

declare -r src_dir1="/ubidata/conf"
declare -r src_dir2="/build/Tools/Calibrator"

dev=`xinput_calibrator --list | mawk -F \" '{print $2}'`
dev_num=`echo -n "${dev}" | sed -n '$='`
if [ "${dev_num}" == "" ]; then
    echo "No input device found"
    exit 1
fi
dev_id=`xinput_calibrator --list | mawk -F "id=" '{print $2}'`

for ((i=1; i<="${dev_num}"; i++)); do
    curr_dev=`echo -n "${dev}" | sed -n "${i}p"`
    curr_dev_mod=`echo -n "${curr_dev}" | sed 's/[^A-Za-z0-9_]/-/g'`
    curr_dev_id=`echo -n "${dev_id}" | sed -n "${i}p"`
    if [ ! -r "${src_dir1}/calibration.conf" ] && [ ! -r "${src_dir2}/${curr_dev_mod}/calibration.conf" ]; then
        echo "No such files: ${src_dir1}/calibration.conf and ${src_dir2}/${curr_dev_mod}/calibration.conf"
        continue
    fi

    for src_dir in "${src_dir1}" "${src_dir2}/${curr_dev_mod}"; do
        if [ ! -r "${src_dir}/calibration.conf" ]; then
            echo "No such file: ${src_dir}/calibration.conf"
            continue
        fi
        # check the output of xinput_calibrator utility
        BEGIN_CHECK="`sed -n  's/\(^Section\).*/\1/p' ${src_dir}/calibration.conf`"
        END_CHECK="`sed -n  's/.*\(EndSection\)\$/\1/p' ${src_dir}/calibration.conf`"
        if [ "${BEGIN_CHECK}" == "" ] || [ "${END_CHECK}" == "" ]; then
            echo "${src_dir}/calibration.conf file is damaged"
            continue
        fi
        evdevAxis=`mawk -F \" '{if ($2 == "Calibration") print $4}' "${src_dir}/calibration.conf"`
        xinput set-prop "${curr_dev_id}" 'Evdev Axis Calibration' ${evdevAxis}
        break
    done
done
