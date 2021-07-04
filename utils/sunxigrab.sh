#!/bin/sh

# Since every process writes into a private dmabuff instead of the fb's memory,
# we can't really rely on just dumping the fb memory region...
# Instead, rely on a disp sysfs attributes that dumps the working buffer to a BMP...

# The path is hardcoded by the kernel, so, make it a tmpfs...
SUNXI_PATH="/mnt/flash"
mkdir -p "${SUNXI_PATH}"
if ! grep -q "^tmpfs ${SUNXI_PATH} tmpfs " /proc/mounts ; then
	# Panel is 1404x1872, so we'll need a bit over 2.5M
	mount -t tmpfs tmpfs ${SUNXI_PATH} -o noatime,size=3M
fi

# Do the thing!
IFS= read -r ret <"/sys/devices/virtual/disp/disp/waveform/get_working_buffer"
if [ "${ret}" = 0 ]; then
	echo "Working buffer dumped to ${SUNXI_PATH}/workingbuffer.bmp"
else
	echo "Failed to dump the working buffer!"
fi

return "${ret}"
