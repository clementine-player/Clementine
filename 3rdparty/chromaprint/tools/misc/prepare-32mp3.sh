#!/bin/bash

for FILE in `ls *-orig.wav`; do
    TMPNEWFILE=`echo $FILE | perl -pe 's/-orig\..*$/-32mp3.mp3/'`
    NEWFILE=`echo $FILE | perl -pe 's/-orig\..*$/-32mp3.wav/'`
	if [ ! -f $NEWFILE ]; then
		rm -f $NEWFILE
		ffmpeg -i $FILE -ab 32000 $TMPNEWFILE
		ffmpeg -i $TMPNEWFILE $NEWFILE
		rm -f $TMPNEWFILE
	fi
done

