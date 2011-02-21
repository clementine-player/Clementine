#!/bin/sh

cd www-bzr || exit
bzr merge || exit
bzr commit --unchanged -m "Merge from main branch" || exit
cd .. || exit

cd www-svn || exit
svn update || exit
cd .. || exit

languages=""
for f in www-bzr/locale/*.po; do
  lang=`basename "$f" .po`
  languages="$languages $lang"

  cp $f www-svn/locale/ || exit
  mkdir -p www-svn/locale/$lang/LC_MESSAGES/

  svn add www-svn/locale/$lang.po www-svn/locale/$lang/
done

sed www-svn/Makefile -i.bak -e "s/LANGUAGES=.*/LANGUAGES=$languages/"

cd www-svn || exit
make || exit
make || exit
cd ../.. || exit

echo "Now change svn-www/data.py to add any new langauges"
