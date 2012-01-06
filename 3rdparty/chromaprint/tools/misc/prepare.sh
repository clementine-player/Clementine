#!/bin/bash

cp orig/*.wav .
./prepare-32mp3.sh
./prepare-64mp3.sh
./prepare-128mp3.sh
./prepare-64wma.sh
./prepare-gain.sh
./prepare-wav.sh
