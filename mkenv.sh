#!/bin/bash

rm -fr env
pyvenv-3.4 ./env
. env/bin/activate
#pip install --upgrade pip
pip install numpy
pip install pyopengl
# pip install pyglfw  didn't work / new api
# pip install pg      unmaintained, python 2, no lib64 search
pip install pyside
