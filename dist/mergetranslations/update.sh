#!/bin/sh

cd bzr || exit
bzr merge || exit
bzr commit --unchanged -m "Merge from main branch" || exit
cd .. || exit

cd svn || exit
svn update || exit
cd .. || exit

cp -v bzr/src/translations/*.po svn/src/translations || exit
svn add svn/src/translations/*.po
cd svn/bin || exit
cmake .. || exit
make -j8 || exit
make -j8 || exit
cd ../.. || exit
