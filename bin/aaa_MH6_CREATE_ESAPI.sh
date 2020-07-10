clear
DATA=`date '+%y%m%d'`
estensione="_commit.mh6"
filename="GPU_v.2.3.4_$DATA$estensione"


FILE_GPU="./EMBEDDED_RELEASE_GPU"
if [ ! -f "$FILE_GPU" ]; then
    echo "$FILE_GPU does not exist"
	cp ./aaa_MH6_CREATE.sh ./error_EMBEDDED_RELEASE_GPU_does_not_exists
	exit 1
fi

FILE_startRhea="../src/startRhea.sh"
if [ ! -f "$FILE_startRhea" ]; then
    echo "$FILE_startRhea does not exist"
	cp ./aaa_MH6_CREATE.sh ./error_startRhea_does_not_exists
	exit 1
fi

#la roba che mi interessa la metto tutta in GPUPackage2019
rm -r ./GPUPackage2019
mkdir ./GPUPackage2019
mkdir ./GPUPackage2019/current
mkdir ./GPUPackage2019/current/lang
cp "$FILE_GPU" ./GPUPackage2019/GPUFusion
cp ./current/lang/*.* ./GPUPackage2019/current/lang
cp ../src/makeRheaServicePack.sh ./GPUPackage2019
rm ./varie/prog/lastUsedLang.txt
cp -r ./varie ./GPUPackage2019
chmod 777 ./GPUPackage2019/GPUFusion
chmod 777 ./GPUPackage2019/makeRheaServicePack.sh
chmod 777 ./GPUPackage2019/current/lang
rm ./GPUPackage2019/varie/prog/js/dev/compiler.jar

#fuori da package c'e' solo startRhea.sh e il miniboot
cp "$FILE_startRhea" ./
chmod 777 ./startRhea.sh

rm ./$filename
tar -czvf $filename ./GPUPackage2019 ./startRhea.sh

rm ./startRhea.sh
rm -r ./GPUPackage2019
