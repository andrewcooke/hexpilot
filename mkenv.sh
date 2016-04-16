#!/bin/bash

rm -fr env
pyvenv-3.5 ./env
. env/bin/activate
pip install --upgrade pip
pip install pyopengl
pip install pg
pip install numpy
