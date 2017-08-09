#!/bin/sh
echo "Firmware damp started.."
dd if=/dev/mtd2 of=/tmp/firmware.bin
echo "Firmware damped on /tmp/firmware.bin"
