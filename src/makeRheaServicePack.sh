#!/bin/bash

DATA=`date '+%y%m%d'`
filename="RHEA_ServicePack.tar.gz"

rm ./$filename
tar -czvf $filename ./current ./GPUFusion ./last_installed
