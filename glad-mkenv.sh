#!/bin/bash

rm -fr env
virtualenv-2.7 env
. env/bin/activate
#pip install glad
pip install --upgrade pip
pip install --upgrade git+https://github.com/dav1dde/glad.git#egg=glad
