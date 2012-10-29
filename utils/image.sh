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

umount /dev/loop0 1>/dev/null 2>&1
losetup -d /dev/loop0 1>/dev/null 2>&1
losetup /dev/loop0 $IMG


########################################
# Formatage
########################################


mkfs -t ext2 /dev/loop0 1>/dev/null 2>&1


########################################
# Ajout des fichiers
########################################


mount -t ext2 /dev/loop0 $MNT 
mkdir $MNT/test
touch $MNT/IwasHere


########################################
# Demontage
#####################################

#umount /dev/loop0
#losetup -d /dev/loop0




