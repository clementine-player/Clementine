#!/bin/bash

# Helper script to install debian dependencies. 
# Run this before create_debian_package.sh to
# install build deps.

sudo apt install devscripts equivs
sudo mk-build-deps --install ../debian/control

