#!/bin/bash

declare -a monitor_geometry
declare -a monitor_name
declare -a monitor_x

declare -r LANG=C
declare -r SCRIPT="${0##*/}"
if [ -f /tmp/err_"${SCRIPT}" ]; then
    sudo rm /tmp/err_"${SCRIPT}"
fi
exec 2>/tmp/err_"${SCRIPT}"
trap 'F_error' ERR
set -o errtrace

###############################################################################
################################## function ###################################
############################################################################
function F_error
{
    #-----------------------------------------
    # usage: < F_error >
    # result: -
    # in process: -
    #
    # var: ${SCRIPT}
    #
    # func: -
    #
    # utils: cat, rm
    #-----------------------------------------
    local I=""
    local stack_size=""

    echo -en "\e[1;31mERROR ${SCRIPT}: "
    unset FUNCNAME[0]
    stack_size="${#FUNCNAME[*]}"
    for I in ${!FUNCNAME[*]}; do
        echo -n "(${FUNCNAME[$I]}:${BASH_LINENO[$I-1]})"
        if [ "${I}" != "${stack_size}" ]; then
            echo -n " <== "
        fi
    done
    echo -en "\n      \e[0m"
    if [ -r /tmp/err_"${SCRIPT}" ]; then
        cat /tmp/err_"${SCRIPT}"
        rm /tmp/err_"${SCRIPT}"
    else
        ${BASH_COMMAND}
    fi
    exit 1
} #end function F_error
###############################################################################
################################# end function ################################
###############################################################################
                                       #
                                       #
                                       #
                                    #######
                                     #####
                                      ###
                                       #
###############################################################################
################################## work #######################################
###############################################################################

trap '' ERR
is_run=`pidof xinput_calibrator`
trap 'F_error' ERR
if [ "${is_run}" != ""  ]; then
    echo "xinput_calibrator is already running." > /tmp/err_"${SCRIPT}"
    false
fi

if [ "${1}" != "" ]; then
    lang="${1}"
elif [ -r /tmp/lang ]; then
    lang=`cat /tmp/lang`
fi
if [ "${lang}" != "" ]; then
    lang="--lang ${lang}"
fi

monitors=`xrandr`
monitors=`echo -n "${monitors}" | sed -n '/ connected/p'`
monitors=`echo -n "${monitors}" | sed 's/primary//g'`

monitors_amount=`echo -n "${monitors}" | sed -n '$='`
if [ "${monitors_amount}" == "" ]; then
    echo "Can not detect monitors" > /tmp/err_"${SCRIPT}"
    false
fi

x_offsets=`echo -n "${monitors}" | mawk '{print $3}'`
x_offsets=`echo -n "${x_offsets}" | mawk -F "+" '{print $2}'`
x_offsets=`echo -n "${x_offsets}" | sort -n`

IFS=$'\n'
for ((i=1; i<="${monitors_amount}"; i++)); do
    offset=`echo -n "${x_offsets}" | sed -n "${i}p"`
    monitor=`echo -n "${monitors}" | sed -n "/+${offset}+/p"`
    monitor_name["${i}"]=`echo -n "${monitor}" | mawk '{print $1}'`
    monitor_geometry["${i}"]=`echo -n "${monitor}" | mawk '{print $3}'`
    # we need to reduce x by 1, otherwise the 'xinput_calibrator' would freeze on the CashDesk.
    monitor_geometry["${i}"]=`echo -n "${monitor_geometry[${i}]}" | mawk -F "x" '{print $1-1"x"$2}'`
    monitor_x["${i}"]=`echo -n "${monitor_geometry["${i}"]}" | mawk -F "x" '{print $1}'`
done
unset IFS

cd ./

###### DEBUG
#trap '' ERR
ids=`./xinput_calibrator --list`
#trap 'F_error' ERR
ids=`echo -n "${ids}" | mawk -F "id=" '{print $2}'`

for ((i=1; i<="${monitors_amount}"; i++)); do
    if [ "${ids}" == "" ]; then
        break
    fi

    if [ "${monitor_x[${i}]}" -le "1024" ]; then
        font="--small"
    else
        font=""
    fi

    ./xinput_calibrator -v ${lang} ${font} --crtc "${monitor_name[${i}]}" --geometry "${monitor_geometry[${i}]}" >> /tmp/calibration"${i}".conf

    # check the output of 'xinput_calibrator' utility
    check_begin="`sed -n  's/\(^Section\).*/\1/p' /tmp/calibration${i}.conf`"
    check_end="`sed -n  's/.*\(EndSection\)\$/\1/p' /tmp/calibration${i}.conf`"

    if [ "${check_begin}" == "" ] || [ "${check_end}" == "" ]; then
        rm /tmp/calibration"${i}".conf
        continue
    fi

    touch_id=`mawk '{if ($1 == "touch_id:") print $4}' /tmp/calibration"${i}".conf`
    ids=`echo -n "${ids}" | mawk -v var="${touch_id}" '{for(i=1; i<=NF; i++) {if ($i != var) print $i}}'`
done

