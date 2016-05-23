#!/bin/bash

. env/bin/activate
python -m glad --spec=gl --api="gl=3.3" --generator c --no-loader --out-path src --local-files

