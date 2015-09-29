#!/bin/bash


USERNAME="sysmon_test"

# Ensure the user exists
function make_user() {
    getent passwd $USERNAME >/dev/null 2>&1

    if [[ $? == 0 ]]; then
        echo "=> Test user '$USERNAME' already exists"
    else
        echo "=> Creating user '$USERNAME'"
        sudo useradd $USERNAME
    fi
}

function remove_user() {
    echo "=> Removing testing user..."
    sudo userdel $USERNAME
}

function run_as_tester() {
    CMD="$1"
    echo "=> Running test command: $CMD"
    sudo su $USERNAME -c "$CMD"
}

function start_sysmon() {
    echo "=> Starting sysmon..."
    make load > make.log
    sudo su -c "echo 1 > /proc/sysmon_toggle"
    echo "=> Setting sysmon uid..."
    uid=$(id -u $USERNAME)
    sudo su -c "echo $uid > /proc/sysmon_uid"
}

function stop_sysmon() {
    echo "=> Stopping sysmon"
    make unload >> make.log
}

function read_sysmon_log() {
    echo "=> Sysmon log contents:"
    sudo cat /proc/sysmon_log # TODO: uncomment when log is implemented
}

make_user
start_sysmon
run_as_tester "$1"
read_sysmon_log
stop_sysmon
remove_user
