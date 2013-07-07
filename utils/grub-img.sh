#!/bin/bash
#
# Create a bootable disk image
#
# Work only with grub 1.98
#
# can be compiled with 
#  ./configure --disable-werror --prefix=/home/grub
#  make && make install
#  ln -s /home/grub/lib/grub /usr/local/lib/grub



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


if [ -z `which dd` ]
then
    echo "No dd found !"
    exit
fi

if [ -z `which fdisk` ]
then
    echo "No fdisk found !"
    exit
fi

if [ -z `which mke2fs` ]
then
    echo "No mke2fs found !"
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



# Raw image creation
dd if=/dev/zero of=$IMG bs=516096c count=8 1>/dev/null 2>&1

# Format
printf "n\np\n1\n2048\n\na\n1\nw" | fdisk -u -C8 -S63 -H16  $IMG 1>/dev/null 2>&1

# Local loop
umount /dev/loop1 1>/dev/null 2>&1
umount /dev/loop0 1>/dev/null 2>&1
losetup -d /dev/loop1 1>/dev/null 2>&1
losetup -d /dev/loop0 1>/dev/null 2>&1
losetup /dev/loop0 $IMG



# Filesystem creation
losetup -o 1048576 /dev/loop1 /dev/loop0
mke2fs -b 1024 /dev/loop1 3000 1>/dev/null 2>&1



# Add Directories
mount -t ext2 /dev/loop1 $MNT 
mkdir $MNT/kern
mkdir $MNT/srv

# Install grub
/home/g4b/grub/sbin/grub-install --modules=part_msdos --root-directory=$MNT /dev/loop0

# Grub boot menu
printf "set timeout=10\nset default=0\n\nmenuentry \"RhinOS\" {\n\tmultiboot /kern/kern\n\tmodule /srv/user1 toto\n\tmodule /srv/user2\n\tmodule /srv/user3\n\tboot\n}\n" > $MNT/boot/grub/grub.cfg


# Clean
umount /dev/loop1 1>/dev/null 2>&1
umount /dev/loop0 1>/dev/null 2>&1
losetup -d /dev/loop1 1>/dev/null 2>&1
losetup -d /dev/loop0 1>/dev/null 2>&1

# tar
tar -czf $IMG.tgz $IMG
rm -f $IMG

# Rights
chmod 666 $IMG.tgz
