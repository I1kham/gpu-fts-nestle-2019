#!/bin/bash

# Absolute path to this script, e.g. /home/user/bin/foo.sh
SCRIPT=$(readlink -f "$0")
# Absolute path this script is in, thus /home/user/bin
SCRIPTPATH=$(dirname "$SCRIPT")
# echo $SCRIPTPATH


cd $SCRIPTPATH
if [ -d "./GPUPackage2019" ]; then
  # Control will enter here if $DIRECTORY exists.
  cp -r ./GPUPackage2019 ../
  rm -r ./GPUPackage2019
fi

cd ../GPUPackage2019
./GPUFusion
