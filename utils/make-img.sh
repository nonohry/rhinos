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
cp ../srv/user_recv $MNT/srv/user_recv
cp ../srv/user_send $MNT/srv/user_send_0
cp ../srv/user_send $MNT/srv/user_send_1

# Change grub config
printf "set timeout=10\nset default=0\n\nmenuentry \"RhinOS\" {\n\tmultiboot /kern/kern\n\tmodule /srv/user_recv receiver\n\tmodule /srv/user_send_0 sender0\n\tmodule /srv/user_send_1 sender1\n\tboot\n}\n" > $MNT/boot/grub/grub.cfg


# Clean
umount /dev/loop0 1>/dev/null 2>&1
losetup -d /dev/loop0 1>/dev/null 2>&1

# Rights
chmod 666 $IMG
