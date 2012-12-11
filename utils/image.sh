#!/bin/bash
#
# Creation d'image disque 
#


########################################
# Arguments
########################################


if [ $# -ne 2 ]
then
    printf "Usage: %s type mountpoint\n\t type = 'floppy' or 'disk'\n" $0 
    exit
fi


########################################
# Constantes
#######################################


TYPE=$1
MNT=$2

IMG=disk.img


########################################
# Verifications
########################################


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

if [ -z `which mkfs` ]
then
    echo "No mkfs found !"
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


if [ $TYPE != "floppy" -a $TYPE != "disk" ]
then
    echo "Type muste be 'floppy' or 'disk'"
    exit
fi


########################################
# Creation d'une galette
########################################


if [ $TYPE == "floppy" ]
then
    dd if=/dev/zero  of=$IMG bs=512 count=2880 1>/dev/null 2>&1
else
    dd if=/dev/zero of=$IMG bs=516096c count=8 1>/dev/null 2>&1
    # Formate le disque
    printf "n\np\n1\n\n\na\n1\nw" | fdisk -u -C8 -S63 -H16  $IMG 1>/dev/null 2>&1
fi


########################################
# Creation de la boucle locale
########################################

umount /dev/loop1 1>/dev/null 2>&1
umount /dev/loop0 1>/dev/null 2>&1
losetup -d /dev/loop1 1>/dev/null 2>&1
losetup -d /dev/loop0 1>/dev/null 2>&1


if [ $TYPE == "floppy" ]
then
    losetup /dev/loop0 $IMG
else
    losetup /dev/loop0 $IMG
fi



########################################
# Formatage
########################################


if [ $TYPE == "floppy" ]
then
    mkfs -t ext2 /dev/loop0 1>/dev/null 2>&1
else
    losetup -o 32256 /dev/loop1 /dev/loop0
    mke2fs -b 1024 /dev/loop1 4000 1>/dev/null 2>&1
fi


########################################
# Ajout des fichiers
########################################


mount -t ext2 /dev/loop1 $MNT 
mkdir $MNT/kern
cp /home/g4b/rhinos/kern/kern $MNT/kern
grub-install --modules=part_msdos --root-directory=$MNT /dev/loop0

printf "set timeout=10\nset default=0\n\nmenuentry \"RhinOS\" {\n\tmultiboot /kern/kern\n\tboot\n}\n" > $MNT/boot/grub/grub.cfg

########################################
# Demontage
#####################################

umount /dev/loop1 1>/dev/null 2>&1
umount /dev/loop0 1>/dev/null 2>&1
losetup -d /dev/loop1 1>/dev/null 2>&1
losetup -d /dev/loop0 1>/dev/null 2>&1

chmod 666 $IMG