#!/bin/bash
set -e

oldpwd=$(pwd)
topdir=$(dirname $0)

cd $topdir
mkdir -p ./build/m4/
autoreconf --force --install --symlink
cd $oldpwd

# https://wiki.debian.org/Multiarch/Tuples
if [[ "$HOSTTYPE" == "x86_64" ]]; then
        ARCHITECTURE_TUPLE=x86_64-linux-gnu
elif [[ "$HOSTTYPE" == "arm" ]]; then
        ARCHITECTURE_TUPLE=arm-linux-gnueabihf
else
        echo "Unknown HOSTTYPE"
        exit 1
fi

if [[ "$1" == "b" ]]; then
        $topdir/configure --prefix=/usr --libdir=/usr/lib/$ARCHITECTURE_TUPLE
        make clean
elif [[ "$1" = "c" ]]; then
        $topdir/configure
        make clean
else
        echo
        echo "----------------------------------------------------------------"
        echo "Initialized build system. For a common configuration please run:"
        echo "----------------------------------------------------------------"
        echo
        echo "$topdir/configure"
        echo
fi
