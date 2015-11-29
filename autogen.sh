#!/bin/sh

set -e

oldpwd=$(pwd)
topdir=$(dirname $0)
cd $topdir

autoreconf --force --install --symlink

if [ -f "$topdir/.config.args" ]; then
        args="$args $(cat $topdir/.config.args)"
fi

cd $oldpwd

if [ "x$1" = "xc" ]; then
        $topdir/configure --enable-debug $args
        make clean
elif [ "x$1" = "xl" ]; then
        $topdir/configure CC=clang $args
        make clean
else
        echo
        echo "----------------------------------------------------------------"
        echo "Initialized build system. For a common configuration please run:"
        echo "----------------------------------------------------------------"
        echo
        echo "$topdir/configure $args"
        echo
fi
