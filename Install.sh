#!/bin/bash

if [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
	# Install python dependencies
	pip install vswhere 1> /dev/null 2> /dev/null
fi
# build third party libraries
python ./Tools/Install.py
