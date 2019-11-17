#!/usr/bin/env bash

print_usage() {
    echo "./xvavro05-fit-ili.sh [--help,--clean,-c]"
}     

if [[ $# > 1 ]] ; then
    echo "Unsupported arguments" >&2
    print_usage
fi

if [[ $1 == "--help" ]]; then
    print_usage
fi

if [[ $1 == "--clean" || $1 == "-c" ]]; then
	umount /mnt/test{1,2}
	rm -r /mnt/test{1,2}
	yes | lvremove /dev/FIT_vg/FIT_lv{1,2}
	vgremove FIT_vg
	pvremove /dev/md{0,1}
 	mdadm --stop /dev/md0
	mdadm --stop /dev/md1
	losetup -D
	rm -f disk{0..3}
	rm -f disk_replace
	rm -f /mnt/test1/big_file
	exit 0
fi

echo "--------------------------------------------------------------------"
dd if=/dev/zero of=disk0 bs=100M count=2
echo "=======> Creating file disk0"

losetup --find disk0

for i in {1..3}; do 
    cp "disk0" disk${i}
    echo "=======> creating file disk${i}" 
    losetup --find disk${i}
done
echo "=======> Four loop devices created"

yes | mdadm --create /dev/md0 --level=stripe --quiet --raid-devices=2 /dev/loop0 /dev/loop1
echo "=======> Devices loop0 and loop1 configured to RAID0"

yes | mdadm --create /dev/md1 --level=mirror --quiet --raid-devices=2 /dev/loop2 /dev/loop3
echo "=======> Devices loop2 and loop3 configured to RAID1"

pvcreate /dev/md{0..1}

vgcreate FIT_vg /dev/md0 /dev/md1 
echo "=======> Volume group FIT_vg created"

lvcreate /dev/FIT_vg -n FIT_lv1 -L 100M
lvcreate /dev/FIT_vg -n FIT_lv2 -L 100M 
echo "=======> Logical volumes FIT_lv{1,2} created"

mkfs.ext4 /dev/FIT_vg/FIT_lv1 
echo "=======> Filesystem ext4 on volume FIT_lv1 created"
mkfs.xfs  /dev/FIT_vg/FIT_lv2
echo "=======> Filesystem xfs on volume FIT_lv2 created"

mkdir /mnt/test{1,2}
mount /dev/FIT_vg/FIT_lv1 /mnt/test1 
echo "=======> Volume FIT_lv1 mounted at /mnt/test1"
mount /dev/FIT_vg/FIT_lv2 /mnt/test2 
echo "=======> Volume FIT_lv2 mounted at /mnt/test2"

lvextend -l +100%FREE /dev/FIT_vg/FIT_lv1
echo "=======> Volume FIT_lv1 resized to max available size" 
resize2fs /dev/FIT_vg/FIT_lv1

df -h

dd if=/dev/zero of=disk_replace bs=100M count=2
losetup loop4 ./disk_replace
echo "=======> File disk_replace and device loop4 created"

dd if=/dev/urandom of=/mnt/test1/big_file bs=30M count=10
echo "=======> File /mnt/test1/big_file created and checksum of this file is:"

sha512sum /mnt/test1/big_file


mdadm --manage /dev/md1 --fail /dev/loop2 --remove /dev/loop2
echo "=======> Device /dev/loop2 removed from raid /dev/md1"
mdadm --manage /dev/md1 --add /dev/loop4
echo "=======> Device /dev/loop2 replaced by /dev/loop4"
sleep 3 #wait for rebuild of RAID

mdadm --detail /dev/md1

