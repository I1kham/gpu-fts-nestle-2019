#!/bin/bash
clear
DATA=`date '+%y%m%d'`
estensione="_commit.mh6"
filename="GPU_FusionBeta1_$DATA$estensione"


FILE_GPU="./bin/EMBEDDED_RELEASE_GPU"
if [ ! -f "$FILE_GPU" ]; then
    echo "$FILE_GPU does not exist"
	cp ./aaa_MH6_CREATE.sh ./error_EMBEDDED_RELEASE_GPU_does_not_exists
	exit 1
fi

FILE_startRhea="./src/startRhea.sh"
if [ ! -f "$FILE_startRhea" ]; then
    echo "$FILE_startRhea does not exist"
	cp ./aaa_MH6_CREATE.sh ./error_startRhea_does_not_exists
	exit 1
fi

cp "$FILE_GPU" ./GPUFusion
cp "$FILE_startRhea" ./

chmod 777 ./GPUFusion
chmod 777 ./startRhea.sh
rm ./$filename
tar -czvf $filename ./GPUFusion ./startRhea.sh

rm ./GPUFusion
rm ./startRhea.sh
#echo $filename
