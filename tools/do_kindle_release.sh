#!/bin/bash -e

# Daisy-chain the three Kindle builds, and package it all
for my_tc in K3 K5 PW2 ; do
	# Setup the x-compile env for this TC
	source ${HOME}/SVN/Configs/trunk/Kindle/Misc/x-compile.sh ${my_tc} env bare

	# And... GO!
	echo "* Launching ${KINDLE_TC} build . . ."

	mkdir -p ${KINDLE_TC}
	make clean

	if [[ "${KINDLE_TC}" == "K3" ]] ; then
		make ${JOBSFLAGS} legacy
	else
		make ${JOBSFLAGS} kindle
	fi

	cp -av Release/fbink ${KINDLE_TC}/fbink
	make clean
done

# Package it...
FBINK_VERSION="$(git describe)"
echo "Here: https://github.com/NiLuJe/FBInk" > "./WHERE_ARE_THE_SOURCES.txt"

rm -f "./FBInk-*-kindle.tar.gz"
tar -cvzf FBInk-${FBINK_VERSION}-kindle.tar.gz README.md LICENSE CREDITS K3 K5 PW2 WHERE_ARE_THE_SOURCES.txt
rm -f "./WHERE_ARE_THE_SOURCES.txt"
rm -rf "./K3" "./K5" "./PW2"
