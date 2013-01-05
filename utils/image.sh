#!/bin/bash
#
# Creation d'image disque 
#


########################################
# Arguments
########################################


if [ $# -ne 1 ]
then
    printf "Usage: %s mountpoint\n" $0 
    exit
fi


########################################
# Constantes
#######################################


MNT=$1

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



########################################
# Creation d'une galette
########################################


dd if=/dev/zero of=$IMG bs=516096c count=8 1>/dev/null 2>&1
# Formate le disque
printf "n\np\n1\n\n\na\n1\nw" | fdisk -u -C8 -S63 -H16  $IMG 1>/dev/null 2>&1



########################################
# Creation de la boucle locale
########################################

umount /dev/loop1 1>/dev/null 2>&1
umount /dev/loop0 1>/dev/null 2>&1
losetup -d /dev/loop1 1>/dev/null 2>&1
losetup -d /dev/loop0 1>/dev/null 2>&1


losetup /dev/loop0 $IMG



########################################
# Formatage
########################################


losetup -o 32256 /dev/loop1 /dev/loop0
mke2fs -b 1024 /dev/loop1 4000 1>/dev/null 2>&1



########################################
# Ajout des fichiers
########################################


mount -t ext2 /dev/loop1 $MNT 
mkdir $MNT/kern
mkdir $MNT/srv
cp ../kern/kern $MNT/kern
cp ../srv/user $MNT/srv/user1
cp ../srv/user $MNT/srv/user2
cp ../srv/user $MNT/srv/user3
grub-install --modules=part_msdos --root-directory=$MNT /dev/loop0

printf "set timeout=10\nset default=0\n\nmenuentry \"RhinOS\" {\n\tmultiboot /kern/kern\n\tmodule /srv/user1\n\tmodule /srv/user2\n\tmodule /srv/user3\n\tboot\n}\n" > $MNT/boot/grub/grub.cfg

########################################
# Demontage
#####################################

umount /dev/loop1 1>/dev/null 2>&1
umount /dev/loop0 1>/dev/null 2>&1
losetup -d /dev/loop1 1>/dev/null 2>&1
losetup -d /dev/loop0 1>/dev/null 2>&1

chmod 666 $IMG