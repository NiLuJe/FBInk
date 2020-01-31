#!/bin/bash -e

# Daisy-chain the three Kindle builds, and package it all
for my_tc in K3 K5 PW2 ; do
	# Setup the x-compile env for this TC
	source ${HOME}/SVN/Configs/trunk/Kindle/Misc/x-compile.sh ${my_tc} env bare

	# And... GO!
	echo "* Launching ${KINDLE_TC} build . . ."

	mkdir -p ${KINDLE_TC}/lib ${KINDLE_TC}/include ${KINDLE_TC}/bin
	make clean

	if [[ "${KINDLE_TC}" == "K3" ]] ; then
		make ${JOBSFLAGS} legacy
	else
		make ${JOBSFLAGS} kindle
	fi

	cp -av Release/fbink ${KINDLE_TC}/bin/fbink
	cp -av fbink.h ${KINDLE_TC}/include/fbink.h

	# We'll want to bundle a shared lib, too, because TCC won't like an LTO archive ;).
	make clean
	if [[ "${KINDLE_TC}" == "K3" ]] ; then
		make ${JOBSFLAGS} sharedlib striplib SHARED=true KINDLE=true LEGACY=true
	else
		make ${JOBSFLAGS} sharedlib striplib SHARED=true KINDLE=true
	fi

	cp -av Release/libfbink.so.1.0.0 ${KINDLE_TC}/lib/libfbink.so.1.0.0
	ln -sf libfbink.so.1.0.0 ${KINDLE_TC}/lib/libfbink.so
	ln -sf libfbink.so.1.0.0 ${KINDLE_TC}/lib/libfbink.so.1

	make clean

	# And we'll want the doom demo, too
	if [[ "${KINDLE_TC}" != "K3" ]] ; then
		make ${JOBSFLAGS} utils KINDLE=true

		cp -av Release/doom ${KINDLE_TC}/bin/doom

		make clean
	fi
done

# Package it...
FBINK_VERSION="$(git describe)"
echo "Here: https://github.com/NiLuJe/FBInk" > "./WHERE_ARE_THE_SOURCES.txt"

mkdir -p ./Kindle
rm -f ./Kindle/FBInk-*-kindle.tar.gz
tar -cvzf Kindle/FBInk-${FBINK_VERSION}-kindle.tar.gz README.md LICENSE CREDITS K3 K5 PW2 WHERE_ARE_THE_SOURCES.txt
rm -f "./WHERE_ARE_THE_SOURCES.txt"
rm -rf "./K3" "./K5" "./PW2"
