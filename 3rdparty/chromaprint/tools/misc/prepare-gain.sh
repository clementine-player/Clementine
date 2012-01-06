#!/bin/bash

for FILE in `ls *-orig.wav`; do
    TMP1FILE=`echo $FILE | perl -pe 's/-orig\..*$/-tmp.wav/'`
    TMP2FILE=`echo $FILE | perl -pe 's/-orig\..*$/-tmp2.wav/'`  
    TMPNEWFILE=`echo $FILE | perl -pe 's/-orig\..*$/-gain.wma/'`
    NEWFILE=`echo $FILE | perl -pe 's/-orig\..*$/-gain.wav/'`
	if [ ! -f $NEWFILE ]; then
		rm -f $NEWFILE $TMP1FILE $TMP2FILE
		ffmpeg -i $FILE $TMP1FILE
		sox $TMP1FILE $TMP2FILE gain -n -10
		ffmpeg -i $TMP2FILE -ab 128000 -acodec wmav2 $TMPNEWFILE
		ffmpeg -i $TMPNEWFILE $NEWFILE
		rm -f $TMP1FILE $TMP2FILE $TMPNEWFILE
	fi
done

