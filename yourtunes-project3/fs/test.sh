#!/bin/bash

# require all commands to succeed
set -e

SONG_NAME="White Wedding"

FS_ROOT="/tmp/ytfs_test"

function quietly() {
    echo "$1" >> test.log
    $1 2>&1 >> test.log
}

echo "=> Init"
sudo umount $FS_ROOT || true
rm -rf $FS_ROOT || true

echo "=> Ensuring song doesn't exist yet..."
rm "/$FS_ROOT/GenresRock/$SONG_NAME.mp3" || true

# run filesystem at $FS_ROOT
echo "=> Building fs binary..."
quietly "make all"
quietly "mkdir -p $FS_ROOT"
echo "=> Running fs..."
./main $FS_ROOT -f & 2>> test.log 1>> test.log

sleep 1

# copy a test song into the fs
echo "=> Copying example song file into fs..."
quietly "cp ./white-wedding.mp3 $FS_ROOT"

# The new file should now show up in the following locations
echo "=> Test new song locations..."
ls "$FS_ROOT/Artists/Billy Idol/$SONG_NAME.mp3" >> test.log
ls "$FS_ROOT/Genres/Rock/$SONG_NAME.mp3" >> test.log

echo "=> Copying file..."
cp "$FS_ROOT/Artists/Billy Idol/$SONG_NAME.mp3" /tmp/cp-test.mp3 >> test.log
MD5_IN="$(md5sum white-wedding.mp3 | cut -d' ' -f1)"
MD5_OUT="$(md5sum /tmp/cp-test.mp3 | cut -d' ' -f1)"
if [[ $MD5_IN != $MD5_OUT ]]; then
    echo "Copying failed :("
    exit 1
fi

echo "=> Success!"

kill %1
