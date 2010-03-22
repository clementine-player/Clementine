#!/bin/sh

name=clementine
url=`svn info | grep URL | head -n1 | awk '{print $2}' | sed 's/\/dist//'`
version=`grep Version $name.spec | head -n1 | awk '{print $2}'`

echo "Checking out $url into $name-$version..."

rm -rf $name-$version
svn export $url $name-$version
tar -cvzf $name-$version.tar.gz $name-$version
tar -cvjf $name-$version.tar.bz2 $name-$version
rm -rf $name-$version
