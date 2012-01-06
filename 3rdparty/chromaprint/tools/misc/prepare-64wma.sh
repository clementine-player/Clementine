#!/bin/bash

for FILE in `ls *-orig.wav`; do
    TMPNEWFILE=`echo $FILE | perl -pe 's/-orig\..*$/-64wma.wma/'`
    NEWFILE=`echo $FILE | perl -pe 's/-orig\..*$/-64wma.wav/'`
	if [ ! -f $NEWFILE ]; then
		rm -f $NEWFILE
		ffmpeg -i $FILE -ab 64000 -acodec wmav2 $TMPNEWFILE
		ffmpeg -i $TMPNEWFILE $NEWFILE
		rm -f $TMPNEWFILE
	fi
done

