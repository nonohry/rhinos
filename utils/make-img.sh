#!/bin/bash
#
# Create a bootable disk image
#


# Usage
if [ $# -ne 1 ]
then
    printf "Usage: %s mountpoint\n" $0 
    exit
fi



# Constants

MNT=$1
IMG=disk.img


# Checks

if [ $(id -g) -ne 0 ]
then
    echo "You must be root !"
    exit
fi


if [ -z `which losetup` ]
then
    echo "No losetup found !"
    exit
fi


if [ ! -d $MNT ]
then
    echo "Mountpoint does not exist !"
    exit
fi

# Untar
tar -xzf $IMG.tgz

# Local loop
umount /dev/loop0 1>/dev/null 2>&1
losetup -d /dev/loop0 1>/dev/null 2>&1
losetup -o 1048576 /dev/loop0 $IMG

# Add files
mount -t ext2 /dev/loop0 $MNT 
cp ../kern/kern $MNT/kern
cp ../srv/user $MNT/srv/user1
cp ../srv/user $MNT/srv/user2
cp ../srv/user $MNT/srv/user3

# Clean
umount /dev/loop0 1>/dev/null 2>&1
losetup -d /dev/loop0 1>/dev/null 2>&1

# Rights
chmod 666 $IMG
