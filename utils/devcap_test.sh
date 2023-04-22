#!/bin/sh
#
# Run a bunch of tests to try to figure out how weird and quirky a device is,
# especially as far as rotation and input are concerned...
#
# NOTE: All the dependencies should ship w/ KoboStuff (except rota):
#       fbgrab, fbink, fbdepth, rota, evtest
##

# Where to?
DEVCAP_LOG="devcap_log.txt"
DEVCAP_PIC="devcap_done.png"

# Prefer local binaries
export PATH=".:${PATH}"
export LD_LIBRARY_PATH=".:${LD_LIBRARY_PATH}"

# Print a header before each test
header() {
	echo "Running test: ${1} . . ."
	echo "** ${1} **" >> "${DEVCAP_LOG}"
	echo "" >> "${DEVCAP_LOG}"
}

# Print a separator between tests
separator() {
	echo "" >> "${DEVCAP_LOG}"
	echo " --------" >> "${DEVCAP_LOG}"
	echo "" >> "${DEVCAP_LOG}"
}

# We'll need the FBInk version
eval "$(fbink -e)"

# Here we go...
echo "* Starting tests on $(date) using FBInk ${FBINK_VERSION}" >> "${DEVCAP_LOG}"

# Start from a blank slate
fbink -q -c -f -w

# Recap the FW & Kernel versions
header "Nickel"
echo "Running FW $(cut -f3 -d',' /mnt/onboard/.kobo/version) on Linux $(uname -r) ($(uname -v))" >> "${DEVCAP_LOG}"
separator

# Dump the NTX HWConfig block
if [ "$(dd if=/dev/mmcblk0 bs=512 skip=1024 count=1 2>/dev/null | head -c 9)" = "HW CONFIG" ] ; then
	header "HWConfig (NXP/Sunxi)"
	ntx_hwconfig -s /dev/mmcblk0 >> "${DEVCAP_LOG}" 2>/dev/null
	separator
elif [ -e "/dev/mmcblk0p6" ] && [ "$(dd if=/dev/mmcblk0p6 bs=512 skip=1 count=1 2>/dev/null | head -c 9)" = "HW CONFIG" ] ; then
	# MTK variant
	header "HWConfig (MTK)"
	ntx_hwconfig ntx_hwconfig -S 1 -p /dev/mmcblk0p6 >> "${DEVCAP_LOG}" 2>/dev/null
	separator
else
	header "Couldn't find a HWConfig tag?!"
	separator
fi

# List input devices
header "Input devices"
ls -lash /dev/input/by-path >> "${DEVCAP_LOG}" 2>/dev/null
separator
ls -lash /dev/input >> "${DEVCAP_LOG}" 2>/dev/null
separator

# List backlights
header "Backlights"
ls -lash /sys/class/backlight >> "${DEVCAP_LOG}" 2>/dev/null
separator

# List LEDs
header "LEDs"
ls -lash /sys/class/leds >> "${DEVCAP_LOG}" 2>/dev/null
separator

header "Power"
ls -lash /sys/class/power_supply >> "${DEVCAP_LOG}" 2>/dev/null
separator

# Start by dumping the full fb state
header "FBGrab"
fbgrab -v /dev/null >> "${DEVCAP_LOG}" 2>&1
separator

# Then say hello
header "FBInk"
fbink -v -w -f -Mm "Hello World" >> "${DEVCAP_LOG}" 2>&1
separator

# Then say hello
header "FBInk state"
fbink -e | tr ';' '\n' >> "${DEVCAP_LOG}" 2>&1
separator

# The rotation madness...
header "Rotation quirks"
rota >> "${DEVCAP_LOG}" 2>&1
separator

# Attempt to go back to UR after the rota madness
header "FBDepth"
fbdepth -r -1 >> "${DEVCAP_LOG}" 2>&1
separator

# Check if we got the canonical mapping right...
header "Canonical rotation mapping"
echo "UpRight" >> "${DEVCAP_LOG}"
fbdepth -R UR >> "${DEVCAP_LOG}" 2>&1
fbink -v -w -m -y -1 -F tewi "↥ UR (@ $(fbdepth -o)) ↥" >> "${DEVCAP_LOG}" 2>&1
fbdepth -c >> "${DEVCAP_LOG}" 2>&1
fbgrab "devcap_UR.png" >/dev/null 2>&1
echo -e "\n" >> "${DEVCAP_LOG}"
echo "ClockWise" >> "${DEVCAP_LOG}"
fbdepth -R CW >> "${DEVCAP_LOG}" 2>&1
fbink -v -w -m -y -1 -F tewi "↥ CW (@ $(fbdepth -o)) ↥" >> "${DEVCAP_LOG}" 2>&1
fbdepth -c >> "${DEVCAP_LOG}" 2>&1
fbgrab "devcap_CW.png" >/dev/null 2>&1
echo -e "\n" >> "${DEVCAP_LOG}"
echo "UpsideDown" >> "${DEVCAP_LOG}"
fbdepth -R UD >> "${DEVCAP_LOG}" 2>&1
fbink -v -w -m -y -1 -F tewi "↥ UD (@ $(fbdepth -o)) ↥" >> "${DEVCAP_LOG}" 2>&1
fbdepth -c >> "${DEVCAP_LOG}" 2>&1
fbgrab "devcap_UD.png" >/dev/null 2>&1
echo -e "\n" >> "${DEVCAP_LOG}"
echo "CounterClockWise" >> "${DEVCAP_LOG}"
fbdepth -R CCW >> "${DEVCAP_LOG}" 2>&1
fbink -v -w -m -y -1 -F tewi "↥ CCW (@ $(fbdepth -o)) ↥" >> "${DEVCAP_LOG}" 2>&1
fbdepth -c >> "${DEVCAP_LOG}" 2>&1
fbgrab "devcap_CCW.png" >/dev/null 2>&1
echo -e "\n" >> "${DEVCAP_LOG}"
separator

# Reset to UpRight
header "Reset to UR"
fbdepth -r -1 >> "${DEVCAP_LOG}" 2>&1
separator

# Check uptime vs. epoch for input timestamps
header "Epoch"
date +%s >> "${DEVCAP_LOG}" 2>&1
separator

header "Uptime"
cat /proc/uptime >> "${DEVCAP_LOG}" 2>&1
separator

# Then ask the user to tap the top-left corner of the screen...
fbink -q -w -Mm -F tewi "⇱ Please tap the top-left corner of the screen ⇱"
# Take a screengrab before the input test, as the tap might trigger a refresh.
fbgrab "${DEVCAP_PIC}" >/dev/null 2>&1
echo "Screengrab saved to ${DEVCAP_PIC}"
echo "See https://github.com/NiLuJe/FBInk/blob/master/utils/devcap_expected_results.png for the expected *on-screen* result".
header "EvTest"
echo "Please tap the top-left corner of the screen in the next 10s!"
(
	if [ -e "/dev/input/by-path/platform-1-0010-event" ] ; then
		evtest "/dev/input/by-path/platform-1-0010-event" >> "${DEVCAP_LOG}" 2>&1
	elif [ -e "/dev/input/by-path/platform-0-0010-event" ] ; then
		evtest "/dev/input/by-path/platform-0-0010-event" >> "${DEVCAP_LOG}" 2>&1
	else
		evtest "/dev/input/event1" >> "${DEVCAP_LOG}" 2>&1
	fi
) &
# Kill it after 10s
sleep 10
kill -TERM $!
separator

# Bye!
echo "" >> "${DEVCAP_LOG}"
echo "* Finished tests on $(date)" >> "${DEVCAP_LOG}"
echo "" >> "${DEVCAP_LOG}"

# Final recap
echo "Results compiled in ${DEVCAP_LOG}"

# Tar it up in the PWD, too
tar -cvzf "${PWD}"/Kobo-DevCap-Results.tar.gz "${DEVCAP_LOG}" devcap_*.png
echo "Results archived in Kobo-DevCap-Results.tar.gz"
