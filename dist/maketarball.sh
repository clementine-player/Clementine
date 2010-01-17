#!/bin/sh

name=clementine
url=http://clementine-player.googlecode.com/svn/trunk
version=`grep Version $name.spec | awk '{print $2}'`

rm -rf $name-$version
svn export $url $name-$version
tar -cvzf $name-$version.tar.gz $name-$version
tar -cvjf $name-$version.tar.bz2 $name-$version
rm -rf $name-$version
