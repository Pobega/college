#! /bin/bash

# Written by Michael Pobega <pobegam@cs.sunyit.edu>
#
# TODO still:
#  - make client send from /path/to/cd.iso to /opt/local/ISO-Images/cd.iso
#  - make sure Wodim options work
#  - setuid for root? (not sure if nessecary)

usage()
{
cat << EOF
usage: $0 <options> disc-image.iso

This script handles the distribution and burning of ISOs for SUNYIT

OPTIONS:
   -h      Show this message
   -f      File to send
   -d      Delete the file after burning (cannot be used alongside -n)
   -g      Which DSH group to send to (defaults to linux)
   -m      Multicast address
   -n      Do not send a file
   -v      Verbose
EOF
}


DELETE=0
GROUP='linux'
SEND=1
M_ADDR=0
while getopts "hdnm:g:f:v" OPTION
do
    case $OPTION in
        h)
            usage
            exit 1
            ;;
        d)
            DELETE=1
            ;;
        n)
            SEND=0
            ;;
        g)
            GROUP=$OPTARG
            ;;
        m)
            M_ADDR=$OPTARG
            ;;
        v)
            VERBOSE=1
            ;;
        f)
            FILE=$OPTARG
            ;;
        ?)
            usage
            exit
            ;;
    esac
done

if [ $FILE ]; then
    echo "Starting udp-receiver on DSH group $GROUP"
    dsh -g $GROUP -- udp-receiver &
    if [ $SEND = 1 ]; then
        if [ $M_ADDR -ne 0 ]; then
            echo "Sending $FILE to $M_ADDR"
            udp-sender --mcast-addr $M_ADDR $FILE
        else
            echo "Sending $FILE to $GROUP"
            udp-sender $FILE
        fi
    fi
    echo "Preparing to burn CDs using Wodim..."
    dsh -g $GROUP -- /usr/bin/cdrecord dev=/dev/scd0 driveropts=noforcespeed fs=14M speed=8 -dao -eject -overburn -v /opt/local/ISO-Images/$FILE; echo "Starting CD burning process. This may take a while..."
    echo "Done"
    if [ $DELETE = 1 ]; then
        echo "Deleting ISO"
        dsh -g $GROUP -- rm /opt/local/ISO-Images/$FILE
    fi
else
    echo "error: usage: $0 <options> disc-image.iso"
fi
