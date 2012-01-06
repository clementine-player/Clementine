#!/bin/bash

for FILE in `ls *.{mp3,wma,ogg}`; do
    NEWFILE=`echo $FILE | perl -pe 's/\..*$/.wav/'`
    rm -f $NEWFILE
    ffmpeg -i $FILE $NEWFILE
    rm -f $FILE
done

