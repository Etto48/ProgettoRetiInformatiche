#!/bin/bash

SERVER="./serv"
SERVER_PID=0
DEVICE="./dev"
DEVICE_PID=0

SPIPE_IN="/tmp/server_test_pipe_in"
DPIPE_IN="/tmp/device_test_pipe_in"

SPIPE_OUT="/tmp/server_test_pipe_out"
DPIPE_OUT="/tmp/device_test_pipe_out"

SERVER_SUCCESS=0
SERVER_TESTS=0
DEVICE_SUCCESS=0
DEVICE_TESTS=0

function cecho()
{
    COLOR_STR=""
    if [[ "$1" = "black" ]]; then
        COLOR_STR="\e[30m"
    elif [[ "$1" = "red" ]]; then
        COLOR_STR="\e[31m"
    elif [[ "$1" = "green" ]]; then
        COLOR_STR="\e[32m"
    elif [[ "$1" = "yellow" ]]; then
        COLOR_STR="\e[33m"
    elif [[ "$1" = "blue" ]]; then
        COLOR_STR="\e[34m"
    elif [[ "$1" = "magenta" ]]; then
        COLOR_STR="\e[35m"
    elif [[ "$1" = "cyan" ]]; then
        COLOR_STR="\e[36m"
    elif [[ "$1" = "white" ]]; then
        COLOR_STR="\e[37m"
    fi
    echo -e "$COLOR_STR$2\e[0m"
}

function test_result()
{
    if [[ "$1" = "device" ]]; then
        let DEVICE_TESTS=$DEVICE_TESTS+1
    elif [[ "$1" = "server" ]]; then
        let SERVER_TESTS=$SERVER_TESTS+1
    fi

    if [[ "$2" = "success" ]]; then
        if [[ "$1" = "device" ]]; then
            cecho green "Test #$DEVICE_TESTS on $1 succeeded"
            let DEVICE_SUCCESS=$DEVICE_SUCCESS+1
        elif [[ "$1" = "server" ]]; then
            cecho green "Test #$SERVER_TESTS on $1 succeeded"
            let SERVER_SUCCESS=$SERVER_SUCCESS+1
        fi
    else
        if [[ "$1" = "device" ]]; then
            cecho red "Test #$DEVICE_TESTS on $1 failed"
        elif [[ "$1" = "server" ]]; then
            cecho red "Test #$SERVER_TESTS on $1 failed"
        fi
    fi
    
}

function test_report()
{
    echo -e "+-----------------------------------------------+"
    echo -e "|                    Report                     |"
    echo -e "+---------------+---------------+---------------+"
    echo -e "|     Module    |   Succeeded   |    Failed     |"
    echo -e "+---------------+---------------+---------------+"
    let SERVER_FAILED=$SERVER_TESTS-$SERVER_SUCCESS
    echo -e "|\t$SERVER\t|\t$SERVER_SUCCESS\t|\t$SERVER_FAILED\t|"
    let DEVICE_FAILED=$DEVICE_TESTS-$DEVICE_SUCCESS
    echo -e "|\t$DEVICE\t|\t$DEVICE_SUCCESS\t|\t$DEVICE_FAILED\t|"
    echo -e "+---------------+---------------+---------------+"
    let TOTAL_SUCCESS=$SERVER_SUCCESS+$DEVICE_SUCCESS
    let TOTAL_FAILED=$SERVER_FAILED+$DEVICE_FAILED
    echo -e "|     Total     |\t$TOTAL_SUCCESS\t|\t$TOTAL_FAILED\t|"
    echo -e "+---------------+---------------+---------------+"
}

#check for executables
if [[ ! -f "$SERVER" ]]; then
    echo "$SERVER not found"
    exit
fi
if [[ ! -f "$DEVICE" ]]; then
    echo "$DEVICE not found"
    exit
fi

#open a pipe for testing server
mkfifo $SPIPE_IN
mkfifo $SPIPE_OUT
#open a pipe for testing device
mkfifo $DPIPE_IN
mkfifo $DPIPE_OUT

#start programs and redirect stdin&stdout
$SERVER < $SPIPE_IN > $SPIPE_OUT &
SERVER_PID=$!
$DEVICE < $DPIPE_IN > $DPIPE_OUT &
DEVICE_PID=$!

touch $SPIPE_IN
touch $DPIPE_IN

sleep 1
############
#   TEST   #
############
#check if processes are still running
if [[ $(head -1 $SPIPE_OUT) = "Server started" ]]; then
    test_result server success
else
    test_result server failure
fi

if [[ $(head -1 $DPIPE_OUT) = "Device started" ]]; then
    test_result device success
else
    test_result device failure
fi


##############
#   REPORT   #
##############
test_report

kill $SERVER_PID 2> /dev/null 
kill $DEVICE_PID 2> /dev/null

#close the server pipe
rm $SPIPE_IN
rm $SPIPE_OUT
#close the device pipe
rm $DPIPE_IN
rm $DPIPE_OUT