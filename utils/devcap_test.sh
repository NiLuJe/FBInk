#!/bin/sh
#
# Run a bunch of tests to try to figure out how weird and quirky a device is,
# especially as far as rotation and input are concerned...
#
# NOTE: All the dependencies should ship w/ KoboStuff (except rota):
#       fbgrab, fbink, fbdepth, rota, evtest
##

# Where to?
DEVCAP_LOG="/mnt/onboard/devcap_log.txt"
DEVCAP_PIC="/mnt/onboard/devcap_done.png"

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
header "HWConfig"
ntx_hwconfig -s /dev/mmcblk0 >> "${DEVCAP_LOG}" 2>/dev/null
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
fbgrab "UR.png" >/dev/null 2>&1
echo -e "\n" >> "${DEVCAP_LOG}"
echo "ClockWise" >> "${DEVCAP_LOG}"
fbdepth -R CW >> "${DEVCAP_LOG}" 2>&1
fbink -v -w -m -y -1 -F tewi "↥ CW (@ $(fbdepth -o)) ↥" >> "${DEVCAP_LOG}" 2>&1
fbdepth -c >> "${DEVCAP_LOG}" 2>&1
fbgrab "CW.png" >/dev/null 2>&1
echo -e "\n" >> "${DEVCAP_LOG}"
echo "UpsideDown" >> "${DEVCAP_LOG}"
fbdepth -R UD >> "${DEVCAP_LOG}" 2>&1
fbink -v -w -m -y -1 -F tewi "↥ UD (@ $(fbdepth -o)) ↥" >> "${DEVCAP_LOG}" 2>&1
fbdepth -c >> "${DEVCAP_LOG}" 2>&1
fbgrab "UD.png" >/dev/null 2>&1
echo -e "\n" >> "${DEVCAP_LOG}"
echo "CounterClockWise" >> "${DEVCAP_LOG}"
fbdepth -R CCW >> "${DEVCAP_LOG}" 2>&1
fbink -v -w -m -y -1 -F tewi "↥ CCW (@ $(fbdepth -o)) ↥" >> "${DEVCAP_LOG}" 2>&1
fbdepth -c >> "${DEVCAP_LOG}" 2>&1
fbgrab "CCW.png" >/dev/null 2>&1
echo -e "\n" >> "${DEVCAP_LOG}"
separator

# Quietly reset to UpRight
fbdepth -q -r -1

# Check uptime vs. epoch for input timestamps
header "Epoch"
date +%s >> "${DEVCAP_LOG}" 2>&1
separator

header "Uptime"
cat /proc/uptime >> "${DEVCAP_LOG}" 2>&1
separator

# Then ask the user to tap the top-left corner of the screen...
fbink -q -w -Mm -F tewi "⇱ Please tap the top-left corner of the screen ⇱"
header "EvTest"
echo "Please tap the top-left corner of the screen in the next 10s!"
(
	evtest /dev/input/event1 >> "${DEVCAP_LOG}" 2>&1
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
fbgrab "${DEVCAP_PIC}" >/dev/null 2>&1
echo "Screengrab saved to ${DEVCAP_PIC}"
