#!/bin/sh

if [ "`uname -s`" = "Darwin" ]; then
	echo "MacOS is not supported now"
	exit 1
fi

case "`uname -m`" in
	x86_64|amd64)
		./Ren-Engine/linux-x86_64
		;;
	i*86)
		./Ren-Engine/linux-i686
		;;
	*)
		echo "Unknown OS"
		echo "You can run need file in dir Ren-Engine"
		exit 1
		;;
esac
