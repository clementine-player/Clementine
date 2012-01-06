#!/bin/bash

for FILE in `ls *-orig.wav`; do
    TMP1FILE=`echo $FILE | perl -pe 's/-orig\..*$/-tmp.wav/'`
    TMPNEWFILE=`echo $FILE | perl -pe 's/-orig\..*$/-resample.wma/'`
    NEWFILE=`echo $FILE | perl -pe 's/-orig\..*$/-resample.wav/'`
	if [ ! -f $NEWFILE ]; then
		rm -f $NEWFILE $TMP1FILE $TMPNEWFILE
		sox $FILE $TMP1FILE rate -l 8k
		ffmpeg -i $TMP1FILE -ab 128000 -acodec wmav2 $TMPNEWFILE
		ffmpeg -i $TMPNEWFILE $NEWFILE
		rm -f $TMP1FILE  $TMPNEWFILE
	fi
done

