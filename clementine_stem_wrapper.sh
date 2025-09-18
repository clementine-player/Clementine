#!/bin/bash
# Wrapper for unbuffered Python execution
cd "/home/stefan/Dokumente/Programmieren lernen/AIMP-Clementine"
source stem_separation_env/bin/activate
export PYTHONUNBUFFERED=1
export PYTHONPATH="/home/stefan/Dokumente/Programmieren lernen/AIMP-Clementine:$PYTHONPATH"
export PYTHONPATH="/usr/lib/python3/dist-packages:$PYTHONPATH"
exec python3 -u clementine_stem_cli.py "$@"
