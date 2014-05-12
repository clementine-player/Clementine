#!/usr/bin/env bash
dire=$1
if [ x$dire == "x" ];then
    dire="src"
fi

find $dire -regex '.*\.\(h\|cpp\|mm\)' -type f -exec ./dist/cpplint.py --root=src {} 2>&1 \;
